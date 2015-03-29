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


#ifndef QOGREWINDOW_H
#define QOGREWINDOW_H

#include <QWindow>

#include <Ogre.h>

#include "SdkQtCameraMan.h"

class QOgreWindow
    : public QWindow
    , public Ogre::FrameListener
{
    Q_OBJECT

public:
    QOgreWindow(QWindow *parent = NULL);
    ~QOgreWindow();

    virtual void render(QPainter *painter);
    virtual void render();
    virtual void initialize();
    virtual void createScene();
#if OGRE_VERSION >= ((2 << 16) | (0 << 8) | 0)
    virtual void createCompositor();
#endif

    void setAnimating(bool animating);

public slots:
    virtual void renderLater();
    virtual void renderNow();

    virtual bool eventFilter(QObject *target, QEvent *event);

signals:
    void entitySelected(Ogre::Entity *entity);

protected:
    Ogre::Root *mOgreRoot;
    Ogre::RenderWindow *mOgreWindow;
    Ogre::SceneManager *mOgreSceneMgr;
    Ogre::Camera *mOgreCamera;
    Ogre::ColourValue mOgreBackground;
    OgreQtBites::SdkQtCameraMan *mCameraMan;

    bool mUpdatePending, mAnimating;

    virtual void keyPressEvent(QKeyEvent *evt);
    virtual void keyReleaseEvent(QKeyEvent *evt);
    virtual void mouseMoveEvent(QMouseEvent *evt);
    virtual void wheelEvent(QWheelEvent *evt);
    virtual void mousePressEvent(QMouseEvent *evt);
    virtual void mouseReleaseEvent(QMouseEvent *evt);
    virtual void exposeEvent(QExposeEvent *evt);
    virtual bool event(QEvent *evt);

    virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

    void log(Ogre::String msg);
    void log(QString msg);
};

#endif /* QOGREWINDOW_H */
