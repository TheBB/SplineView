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

#include <iostream>

#include <QKeyEvent>
#include <QMouseEvent>

#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreFrameListener.h"
#include "OgreQuaternion.h"

namespace OgreQtBites
{

class SdkQtCameraMan
{
public:

    SdkQtCameraMan(Ogre::Camera *cam)
        : mCamera(NULL)
        , mTarget(NULL)
        , mOrbiting(false)
    {
        mCamera = cam;
        mTarget = mCamera->getSceneManager()->getRootSceneNode();
        mCamera->setAutoTracking(true, mTarget);
        mCamera->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
        setYawPitchDist(Ogre::Degree(30), Ogre::Degree(30), 30);
    }

    virtual ~SdkQtCameraMan() {}

    virtual void setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist)
    {
        mCamera->setPosition(mTarget->_getDerivedPosition());
        mCamera->setOrientation(Ogre::Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
        mCamera->yaw(-yaw);
        mCamera->pitch(Ogre::Degree(90) - pitch);
        mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
    }

    virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        return true;
    }

    virtual void injectKeyDown(const QKeyEvent &evt)
    {
    }

    virtual void injectKeyUp(const QKeyEvent &evt)
    {
    }

    virtual void injectMouseMove(float relX, float relY)
    {
        if (mOrbiting)
        {
            Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();
            mCamera->setPosition(mTarget->_getDerivedPosition());
            mCamera->yaw(Ogre::Degree(-relX * 360));
            mCamera->pitch(Ogre::Degree(-relY * 180));
            mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
        }
    }

    virtual void injectWheelMove(const QWheelEvent &evt)
    {
        int relZ = evt.delta();
        Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();

        if (relZ != 0)
            mCamera->moveRelative(Ogre::Vector3(0, 0, -relZ * 0.0008f * dist));
    }

    virtual void injectMouseDown(const QMouseEvent &evt)
    {
        if (evt.button() == Qt::RightButton)
            mOrbiting = true;
    }

    virtual void injectMouseUp(const QMouseEvent &evt)
    {
        if (evt.button() == Qt::RightButton)
            mOrbiting = false;
    }

protected:
    
    Ogre::Camera *mCamera;
    Ogre::SceneNode *mTarget;

    bool mOrbiting;
};

}

#endif /* SDKQTCAMERAMAN_H */
