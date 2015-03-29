/*
 * Copyright (C) 2015 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of SplineView.
 *
 * SplineView is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * SplineView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with SplineView. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using SplineView.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the SplineView library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

#include <iostream>

#include <QApplication>

#include "QOgreWindow.h"


QOgreWindow::QOgreWindow(QWindow *parent)
    : QWindow(parent)
    , mUpdatePending(false)
    , mAnimating(false)
    , mOgreRoot(NULL)
    , mOgreWindow(NULL)
    , mOgreCamera(NULL)
    , mCameraMan(NULL)
{
    setAnimating(true);
    installEventFilter(this);
    mOgreBackground = Ogre::ColourValue(0.0f, 0.5f, 1.0f);
}


QOgreWindow::~QOgreWindow()
{
    if (mCameraMan)
        delete mCameraMan;
    delete mOgreRoot;
}


void QOgreWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}


void QOgreWindow::initialize()
{
#ifdef _MSC_VER
    mOgreRoot = new Ogre::Root(Ogre::String("ogre/plugins" OGRE_BUILD_SUFFIX ".cfg"));
#else
    mOgreRoot = new Ogre::Root(Ogre::String("ogre/plugins.cfg"));
#endif

    Ogre::ConfigFile ogreConfig;

    const Ogre::RenderSystemList &rsList = mOgreRoot->getAvailableRenderers();
    for (auto rs : rsList)
        std::cout << "---rs: " << rs->getName() << std::endl;
    Ogre::RenderSystem *rs = NULL;

    Ogre::StringVector renderOrder;
#if defined(Q_OS_WIN)
    renderOrder.push_back("Direct3D9");
    renderOrder.push_back("Direct3D11");
#endif
    renderOrder.push_back("OpenGL");
    renderOrder.push_back("OpenGL 3+");

    for (auto s : renderOrder)
    {
        for (auto renderer : rsList)
            if (renderer->getName().find(s) != Ogre::String::npos)
            {
                rs = renderer;
                break;
            }
        if (rs != NULL)
            break;
    }

    if (rs == NULL && mOgreRoot->restoreConfig() && !mOgreRoot->showConfigDialog())
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
                    "Abort render system configuration",
                    "QOgreWindow::initialize");

    std::cout << "---found renderer" << std::endl;
    std::cout << rs << std::endl;
    QString dimensions = QString("%1 x %2").arg(this->width()).arg(this->height());
    rs->setConfigOption("Video Mode", dimensions.toStdString());
    rs->setConfigOption("Full Screen", "No");
    rs->setConfigOption("VSync", "Yes");
    mOgreRoot->setRenderSystem(rs);
    mOgreRoot->initialise(false);

    Ogre::NameValuePairList parameters;
    if (rs->getName().find("GL") <= rs->getName().size())
        parameters["currentGLContext"] = Ogre::String("false");

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    parameters["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(this->winId()));
    parameters["parentWindowHandle"] = Ogre::StringConverter::toString((size_t)(this->winId()));
#else
    parameters["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(this->winId()));
    parameters["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(this->winId()));
#endif

#if defined(Q_OS_MAC)
    parameters["macAPI"] = "cocoa";
    parameters["macAPICocoaUseNSView"] = "true";
#endif

    mOgreWindow = mOgreRoot->createRenderWindow(
        "Qt Window", this->width(), this->height(), false, &parameters);
    mOgreWindow->setVisible(true);

#if OGRE_VERSION >= ((2 << 16) | (0 << 8) | 0)
    const size_t numThreads = std::max<int>(1, Ogre::PlatformInformation::getNumLogicalCores());
    Ogre::InstancingThreadedCullingMethod threadedCullingMethod = Ogre::INSTANCING_CULLING_SINGLETHREAD;
    if (numThreads > 1)
        threadedCullingMethod = Ogre::INSTANCING_CULLING_THREADED;
    mOgreSceneMgr = mOgreRoot->createSceneManager(Ogre::ST_GENERIC, numThreads, threadedCullingMethod);
#else
    mOgreSceneMgr = mOgreRoot->createSceneManager(Ogre::ST_GENERIC);
#endif

    mOgreCamera = mOgreSceneMgr->createCamera("MainCamera");
    mOgreCamera->setPosition(Ogre::Vector3(0.0f, 0.0f, 10.0f));
    mOgreCamera->lookAt(Ogre::Vector3(0.0f, 0.0f, -300.0f));
    mOgreCamera->setNearClipDistance(0.1f);
    mOgreCamera->setFarClipDistance(200.0f);
    mCameraMan = new OgreQtBites::SdkQtCameraMan(mOgreCamera);

#if OGRE_VERSION >= ((2 << 16) | (0 << 8) | 0)
    createCompositor();
#else
    Ogre::Viewport *viewport = mOgreWindow->addViewport(mOgreCamera);
    viewport->setBackgroundColour(mOgreBackground);
#endif

    mOgreCamera->setAspectRatio(Ogre::Real(mOgreWindow->getWidth()) /
                                Ogre::Real(mOgreWindow->getHeight()));
    mOgreCamera->setAutoAspectRatio(true);

    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    createScene();

    mOgreRoot->addFrameListener(this);
}


void QOgreWindow::createScene()
{
    mOgreSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

#if OGRE_VERSION >= ((2 << 16) | (0 << 8) | 0)
    Ogre::Entity *sphereMesh = mOgreSceneMgr->createEntity(Ogre::SceneManager::PT_SPHERE);
#else
    Ogre::Entity *sphereMesh = mOgreSceneMgr->createEntity("Sphere", Ogre::SceneManager::PT_SPHERE);
#endif

    Ogre::SceneNode *childSceneNode = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode();
    childSceneNode->attachObject(sphereMesh);

    Ogre::MaterialPtr sphereMaterial = Ogre::MaterialManager::getSingleton().create(
        "SphereMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);

    sphereMaterial->getTechnique(0)->getPass(0)->setAmbient(0.1f, 0.1f, 0.1f);
    sphereMaterial->getTechnique(0)->getPass(0)->setDiffuse(0.2f, 0.2f, 0.2f, 1.0f);
    sphereMaterial->getTechnique(0)->getPass(0)->setSpecular(0.9f, 0.9f, 0.9f, 1.0f);

    sphereMesh->setMaterialName("SphereMaterial");
    childSceneNode->setPosition(Ogre::Vector3(0.0f, 0.0f, 0.0f));
    childSceneNode->setScale(Ogre::Vector3(0.01f, 0.01f, 0.01f));

#if OGRE_VERSION >= ((2 << 16) | (0 << 8) | 0)
    Ogre::SceneNode *lightNode = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode();
    Ogre::Light *light = mOgreSceneMgr->createLight();
    lightNode->attachObject(light);
    lightNode->setPosition(20.0f, 80.0f, 50.0f);
#else
    Ogre::Light *light = mOgreSceneMgr->createLight("Light");
    light->setPosition(20.0f, 80.0f, 50.0f);
#endif
}


#if OGRE_VERSION >= ((2 << 16) | (0 << 8) | 0)
void QOgreWindow::createCompositor()
{
    Ogre::CompositorManager2 *compMan = mOgreRoot->getCompositorManager2();
    const Ogre::String workspaceName = "default scene workspace";
    const Ogre::IdString workspaceNameHash = workspaceName;
    compMan->createBasicWorkspaceDef(workspaceName, mOgreBackground);
    compMan->addWorkspace(mOgreSceneMgr, mOgreWindow, mOgreCamera, workspaceNameHash, true);
}
#endif


void QOgreWindow::render()
{
    Ogre::WindowEventUtilities::messagePump();
    mOgreRoot->renderOneFrame();
}


void QOgreWindow::renderLater()
{
    if (!mUpdatePending)
    {
        mUpdatePending = true;
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}


bool QOgreWindow::event(QEvent *evt)
{
    switch (evt->type())
    {
    case QEvent::UpdateRequest:
        mUpdatePending = false;
        renderNow();
        return true;

    default:
        return QWindow::event(evt);
    }
}


void QOgreWindow::exposeEvent(QExposeEvent *evt)
{
    Q_UNUSED(evt);

    if (isExposed())
        renderNow();
}


void QOgreWindow::renderNow()
{
    if (!isExposed())
        return;

    if (mOgreRoot == NULL)
        initialize();

    render();


    if (mAnimating)
        renderLater();
}


bool QOgreWindow::eventFilter(QObject *target, QEvent *evt)
{
    if (target == this && evt->type() == QEvent::Resize && isExposed() && mOgreWindow != NULL)
        mOgreWindow->resize(this->width(), this->height());
    return false;
}


void QOgreWindow::keyPressEvent(QKeyEvent *evt)
{
    if (mCameraMan)
        mCameraMan->injectKeyDown(*evt);
}


void QOgreWindow::keyReleaseEvent(QKeyEvent *evt)
{
    if (mCameraMan)
        mCameraMan->injectKeyUp(*evt);
}


void QOgreWindow::mouseMoveEvent(QMouseEvent *evt)
{
    static int lastX = evt->x();
    static int lastY = evt->y();
    int relX = evt->x() - lastX;
    int relY = evt->y() - lastY;
    lastX = evt->x();
    lastY = evt->y();

    if (mCameraMan && (evt->buttons() & Qt::LeftButton))
        mCameraMan->injectMouseMove(relX, relY);
}


void QOgreWindow::wheelEvent(QWheelEvent *evt)
{
    if (mCameraMan)
        mCameraMan->injectWheelMove(*evt);
}


void QOgreWindow::mousePressEvent(QMouseEvent *evt)
{
    if (mCameraMan)
        mCameraMan->injectMouseDown(*evt);
}


void QOgreWindow::mouseReleaseEvent(QMouseEvent *evt)
{
    if (mCameraMan)
        mCameraMan->injectMouseUp(*evt);

    QPoint pos = evt->pos();
    Ogre::Ray mouseRay = mOgreCamera->getCameraToViewportRay(
        (Ogre::Real)pos.x() / mOgreWindow->getWidth(),
        (Ogre::Real)pos.y() / mOgreWindow->getHeight());
    Ogre::RaySceneQuery *query = mOgreSceneMgr->createRayQuery(mouseRay);
    query->setSortByDistance(true);
    Ogre::RaySceneQueryResult result = query->execute();
    for (size_t i = 0; i < result.size(); i++)
        if (result[i].movable && result[i].movable->getMovableType().compare("Entity") == 0)
            emit entitySelected((Ogre::Entity*)result[i].movable);
    mOgreSceneMgr->destroyQuery(query);
}


void QOgreWindow::setAnimating(bool animating)
{
    mAnimating = animating;
    if (animating)
        renderLater();
}


bool QOgreWindow::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
    if (mCameraMan)
        mCameraMan->frameRenderingQueued(evt);
    return true;
}


void QOgreWindow::log(Ogre::String msg)
{
    if (Ogre::LogManager::getSingletonPtr() != NULL)
        Ogre::LogManager::getSingletonPtr()->logMessage(msg);
}


void QOgreWindow::log(QString msg)
{
    log(Ogre::String(msg.toStdString().c_str()));
}
