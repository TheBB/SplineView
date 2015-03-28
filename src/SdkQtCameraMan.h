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


#ifndef SDKQTCAMERAMAN_H
#define SDKQTCAMERAMAN_H

#include <QKeyEvent>
#include <QMouseEvent>

#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreFrameListener.h"

namespace OgreQtBites
{

enum CameraStyle { CS_FREELOOK, CS_ORBIT, CS_MANUAL };

class SdkQtCameraMan
{
public:

    SdkQtCameraMan(Ogre::Camera *cam)
        : mCamera(NULL)
        , mTarget(NULL)
        , mTopSpeed(150)
        , mVelocity(Ogre::Vector3::ZERO)
        , mOrbiting(false)
        , mZooming(false)
        , mGoingForward(false)
        , mGoingBack(false)
        , mGoingLeft(false)
        , mGoingRight(false)
        , mGoingUp(false)
        , mGoingDown(false)
        , mFastMove(false)

    {
        setCamera(cam);
        setStyle(CS_FREELOOK);
    }

    virtual ~SdkQtCameraMan() {}

    virtual void setCamera(Ogre::Camera *cam) { mCamera = cam; }
    virtual Ogre::Camera *getCamera() { return mCamera; }

    // Only for orbit style
    virtual void setTarget(Ogre::SceneNode *target)
    {
        if (target != mTarget)
        {
            mTarget = target;
            if (target)
            {
                setYawPitchDist(Ogre::Degree(0), Ogre::Degree(15), 150);
                mCamera->setAutoTracking(true, mTarget);
            }
            else
                mCamera->setAutoTracking(false);
        }
    }

    virtual Ogre::SceneNode *getTarget() { return mTarget; }

    // Only for orbit style
    virtual void setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist)
    {
        mCamera->setPosition(mTarget->_getDerivedPosition());
        mCamera->setOrientation(mTarget->_getDerivedOrientation());
        mCamera->yaw(yaw);
        mCamera->pitch(-pitch);
        mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
    }

    virtual void setTopSpeed(Ogre::Real topSpeed) { mTopSpeed = topSpeed; }
    virtual Ogre::Real getTopSpeed() { return mTopSpeed; }

    virtual void setStyle(CameraStyle style)
    {
        if (mStyle != CS_ORBIT && style == CS_ORBIT)
        {
            setTarget(mTarget ? mTarget : mCamera->getSceneManager()->getRootSceneNode());
            mCamera->setFixedYawAxis(true);
            manualStop();
            setYawPitchDist(Ogre::Degree(0), Ogre::Degree(15), 150);
        }
        else if (mStyle != CS_FREELOOK && style == CS_FREELOOK)
        {
            mCamera->setAutoTracking(false);
            mCamera->setFixedYawAxis(true);
        }
        else if (mStyle != CS_MANUAL && style == CS_MANUAL)
        {
            mCamera->setAutoTracking(false);
            manualStop();
        }

        mStyle = style;
    }

    virtual CameraStyle getStyle() { return mStyle; }

    // Manually stops the camera in freelook mode
    virtual void manualStop()
    {
        if (mStyle != CS_FREELOOK)
            return;

        mGoingForward = false;
        mGoingBack = false;
        mGoingLeft = false;
        mGoingRight = false;
        mGoingUp = false;
        mGoingDown = false;
        mVelocity = Ogre::Vector3::ZERO;
    }
    
    virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        if (mStyle != CS_FREELOOK)
            return true;

        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        if (mGoingForward) accel += mCamera->getDirection();
        if (mGoingBack) accel -= mCamera->getDirection();
        if (mGoingRight) accel += mCamera->getRight();
        if (mGoingLeft) accel -= mCamera->getRight();
        if (mGoingUp) accel += mCamera->getUp();
        if (mGoingDown) accel -= mCamera->getUp();

        Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
        if (accel.squaredLength() != 0)
        {
            accel.normalise();
            mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
        }
        else
            mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

        Ogre::Real eps = std::numeric_limits<Ogre::Real>::epsilon();

        if (mVelocity.squaredLength() > topSpeed * topSpeed)
        {
            mVelocity.normalise();
            mVelocity *= topSpeed;
        }
        else if (mVelocity.squaredLength() < eps * eps)
            mVelocity = Ogre::Vector3::ZERO;

        if (mVelocity != Ogre::Vector3::ZERO)
            mCamera->move(mVelocity * evt.timeSinceLastFrame);

        return true;
    }

    virtual void injectKeyDown(const QKeyEvent &evt)
    {
        if (mStyle != CS_FREELOOK)
            return;

        switch (evt.key())
        {
        case Qt::Key_W: case Qt::Key_Up: mGoingForward = true; break;
        case Qt::Key_S: case Qt::Key_Down: mGoingBack = true; break;
        case Qt::Key_A: case Qt::Key_Left: mGoingLeft = true; break;
        case Qt::Key_D: case Qt::Key_Right: mGoingRight = true; break;
        case Qt::Key_PageUp: mGoingUp = true; break;
        case Qt::Key_PageDown: mGoingDown = true; break;
        case Qt::Key_Shift: mFastMove = true; break;
        }
    }

    virtual void injectKeyUp(const QKeyEvent &evt)
    {
        if (mStyle != CS_FREELOOK)
            return;

        switch (evt.key())
        {
        case Qt::Key_W: case Qt::Key_Up: mGoingForward = false; break;
        case Qt::Key_S: case Qt::Key_Down: mGoingBack = false; break;
        case Qt::Key_A: case Qt::Key_Left: mGoingLeft = false; break;
        case Qt::Key_D: case Qt::Key_Right: mGoingRight = false; break;
        case Qt::Key_PageUp: mGoingUp = false; break;
        case Qt::Key_PageDown: mGoingDown = false; break;
        case Qt::Key_Shift: mFastMove = false; break;
        }
    }

    virtual void injectMouseMove(int relX, int relY)
    {
        if (mStyle == CS_ORBIT)
        {
            Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();

            if (mOrbiting)
            {
                mCamera->setPosition(mTarget->_getDerivedPosition());
                mCamera->yaw(Ogre::Degree(-relX * 0.025f));
                mCamera->pitch(Ogre::Degree(-relY * 0.025f));
                mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
            }
            else if (mZooming)
                mCamera->moveRelative(Ogre::Vector3(0, 0, relY * 0.004f * dist));
        }
        else if (mStyle == CS_FREELOOK)
        {
            mCamera->yaw(Ogre::Degree(-relX * 0.15f));
            mCamera->yaw(Ogre::Degree(-relY * 0.15f));
        }
    }

    virtual void injectWheelMove(const QWheelEvent &evt)
    {
        if (mStyle != CS_ORBIT)
            return;

        int relZ = evt.delta();
        Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();

        if (relZ != 0)
            mCamera->moveRelative(Ogre::Vector3(0, 0, -relZ * 0.0008f * dist));
    }

    virtual void injectMouseDown(const QMouseEvent &evt)
    {
        if (mStyle != CS_ORBIT)
            return;

        if (evt.buttons() & Qt::LeftButton)
            mOrbiting = true;
        else if (evt.buttons() & Qt::LeftButton)
            mZooming = true;
    }

    virtual void injectMouseUp(const QMouseEvent &evt)
    {
        if (mStyle != CS_ORBIT)
            return;

        if (evt.buttons() & Qt::LeftButton)
            mOrbiting = false;
        else if (evt.buttons() & Qt::LeftButton)
            mZooming = false;
    }

protected:
    
    Ogre::Camera *mCamera;
    Ogre::SceneNode *mTarget;

    CameraStyle mStyle;

    Ogre::Real mTopSpeed;
    Ogre::Vector3 mVelocity;
    bool mOrbiting, mZooming;
    bool mGoingForward, mGoingBack, mGoingLeft, mGoingRight, mGoingUp, mGoingDown;
    bool mFastMove;
};

}

#endif /* SDKQTCAMERAMAN_H */
