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


#include <sstream>

#include "MainWindow.h"

#include "QOgreWindow.h"
#include "main.h"


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    std::ostringstream ss;
    ss << "SINTEF SplineView "
       << SV_VERSION_MAJOR << "."
       << SV_VERSION_MINOR << "."
       << SV_VERSION_PATCH;
    setWindowTitle(QString(ss.str().c_str()));

    QOgreWindow *ogreWindow = new QOgreWindow();
    QWidget *renderingContainer = QWidget::createWindowContainer(ogreWindow);
    setCentralWidget(renderingContainer);
}


MainWindow::~MainWindow() {}
