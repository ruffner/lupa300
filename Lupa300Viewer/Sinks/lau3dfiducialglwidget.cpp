/*********************************************************************************
 *                                                                               * 
 * Copyright (c) 2017, Dr. Daniel L. Lau                                         * 
 * All rights reserved.                                                          * 
 *                                                                               * 
 * Redistribution and use in source and binary forms, with or without            * 
 * modification, are permitted provided that the following conditions are met:   * 
 * 1. Redistributions of source code must retain the above copyright             * 
 *    notice, this list of conditions and the following disclaimer.              * 
 * 2. Redistributions in binary form must reproduce the above copyright          * 
 *    notice, this list of conditions and the following disclaimer in the        * 
 *    documentation and/or other materials provided with the distribution.       * 
 * 3. All advertising materials mentioning features or use of this software      * 
 *    must display the following acknowledgement:                                * 
 *    This product includes software developed by the <organization>.            * 
 * 4. Neither the name of the <organization> nor the                             * 
 *    names of its contributors may be used to endorse or promote products       * 
 *    derived from this software without specific prior written permission.      * 
 *                                                                               * 
 * THIS SOFTWARE IS PROVIDED BY Dr. Daniel L. Lau ''AS IS'' AND ANY              * 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     * 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        * 
 * DISCLAIMED. IN NO EVENT SHALL Dr. Daniel L. Lau BE LIABLE FOR ANY             * 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    * 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  * 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND   * 
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    * 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS * 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                  * 
 *                                                                               * 
 *********************************************************************************/

#include "lau3dfiducialglwidget.h"
#include "locale.h"

QString LAU3DFiducialWidget::lastDirectoryString;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFiducialTool::LAUFiducialTool(QWidget *parent) : QWidget(parent)
{
    this->setWindowFlags(Qt::Tool);
    this->setWindowTitle(QString("Fiducials"));
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);

    table = new QTableWidget();
    table->setRowCount(0);
    table->setColumnCount(6);
    table->setFixedWidth(620);
    table->setColumnWidth(0,100);
    table->setColumnWidth(1,100);
    table->setColumnWidth(2,100);
    table->setColumnWidth(3,100);
    table->setColumnWidth(4,100);
    table->setColumnWidth(5,100);
    table->setAlternatingRowColors(true);
    table->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(QString("X")));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(QString("Y")));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem(QString("Z")));
    table->setHorizontalHeaderItem(3, new QTableWidgetItem(QString("R")));
    table->setHorizontalHeaderItem(4, new QTableWidgetItem(QString("G")));
    table->setHorizontalHeaderItem(5, new QTableWidgetItem(QString("B")));

    this->layout()->addWidget(table);
    this->setFixedWidth(620);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialTool::onFiducialsChanged(QVector3D point, int index, QVector3D color)
{
    if (table){
        table->item(index, 0)->setText(QString("%1").arg(point.x(), 0, 'f', 3));
        table->item(index, 1)->setText(QString("%1").arg(point.y(), 0, 'f', 3));
        table->item(index, 2)->setText(QString("%1").arg(point.z(), 0, 'f', 3));
        table->item(index, 3)->setText(QString("%1").arg(color.x(), 0, 'f', 3));
        table->item(index, 4)->setText(QString("%1").arg(color.y(), 0, 'f', 3));
        table->item(index, 5)->setText(QString("%1").arg(color.z(), 0, 'f', 3));
        table->selectRow(index);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialTool::onFiducialsChanged(QList<QVector3D> points, QList<QVector3D> colors)
{
    table->setRowCount(points.count());
    for (int r=0; r<points.count(); r++){
        table->setItem(r, 0, new QTableWidgetItem(QString("%1").arg(points.at(r).x(), 0, 'f', 3)));
        table->setItem(r, 1, new QTableWidgetItem(QString("%1").arg(points.at(r).y(), 0, 'f', 3)));
        table->setItem(r, 2, new QTableWidgetItem(QString("%1").arg(points.at(r).z(), 0, 'f', 3)));
        table->setItem(r, 3, new QTableWidgetItem(QString("%1").arg(colors.at(r).x(), 0, 'f', 3)));
        table->setItem(r, 4, new QTableWidgetItem(QString("%1").arg(colors.at(r).y(), 0, 'f', 3)));
        table->setItem(r, 5, new QTableWidgetItem(QString("%1").arg(colors.at(r).z(), 0, 'f', 3)));
        table->setVerticalHeaderItem(r, new QTableWidgetItem(QString("%1").arg(QChar(65+r))));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DFiducialWidget::LAU3DFiducialWidget(LAUScan scan, QWidget *parent) : QWidget(parent), filenameString(QString()), scanWidget(NULL)
{
    this->setFocusPolicy(Qt::StrongFocus);
    QSettings settings;
    lastDirectoryString = settings.value(QString("LAU3DFiducialWidget::lastDirectoryString"), QStandardPaths::displayName(QStandardPaths::DocumentsLocation)).toString();

    this->setLayout(new QHBoxLayout());

    fiducialLabel = new LAUFiducialLabel(QImage(":/Images/sample.tif"));
    connect(fiducialLabel, SIGNAL(emitPointMoved(QString,int,int)), this, SLOT(onUpdatePoint(QString,int,int)));
    connect(fiducialLabel, SIGNAL(emitDoubleClick(int,int)), this, SLOT(onAddItem(int,int)));

    QWidget *widgetA = new QWidget();
    widgetA->setLayout(new QHBoxLayout);
    widgetA->layout()->setContentsMargins(0,0,0,0);
    widgetA->layout()->setSpacing(0);
    widgetA->layout()->addWidget(fiducialLabel);
    widgetA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(widgetA);

    newButton = new QToolButton();
    newButton->setText(QString("add"));
    newButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(newButton, SIGNAL(clicked()), this, SLOT(onAddItem()));

    delButton = new QToolButton();
    delButton->setText(QString("delete"));
    delButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(delButton, SIGNAL(clicked()), this, SLOT(onDeleteItem()));

    upButton  = new QToolButton();
    upButton->setText(QString("up"));
    upButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(upButton, SIGNAL(clicked()), this, SLOT(onMoveUpItem()));

    dwnButton = new QToolButton();
    dwnButton->setText(QString("down"));
    dwnButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(dwnButton, SIGNAL(clicked()), this, SLOT(onMoveDownItem()));

    widgetA = new QWidget();
    widgetA->setLayout(new QHBoxLayout);
    widgetA->setFixedWidth(328);
    widgetA->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    widgetA->layout()->setContentsMargins(0,0,0,0);
    widgetA->layout()->setSpacing(0);
    widgetA->layout()->addWidget(newButton);
    widgetA->layout()->addWidget(delButton);
    widgetA->layout()->addWidget(upButton);
    widgetA->layout()->addWidget(dwnButton);

    table = new QTableWidget();
    table->setRowCount(0);
    table->setColumnCount(3);
    table->setFixedWidth(328);
    table->setColumnWidth(0,100);
    table->setColumnWidth(1,100);
    table->setColumnWidth(2,100);
    table->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    table->setHorizontalHeaderItem(0, new QTableWidgetItem(QString("X")));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(QString("Y")));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem(QString("Z")));
    connect(table, SIGNAL(currentCellChanged(int,int,int,int)), fiducialLabel, SLOT(setCurrentPoint(int,int,int,int)));
    connect(fiducialLabel, SIGNAL(emitCurrentPointChanged(int)), table, SLOT(selectRow(int)));

    QWidget *widgetB = new QWidget();
    widgetB->setLayout(new QVBoxLayout);
    widgetB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetB->layout()->setContentsMargins(0,0,0,0);
    widgetB->layout()->setSpacing(0);
    widgetB->layout()->addWidget(table);
    widgetB->layout()->addWidget(widgetA);

    this->layout()->addWidget(widgetB);
    if (scan.isValid()){
        localScan = scan;
        fiducialLabel->setImage(localScan.preview(localScan.size()));
        scanFileString = scan.filename();
        this->setWindowTitle(scanFileString);
    } else {
        this->load();
    }
    displayScan();
    table->setFocusPolicy(Qt::NoFocus);
    fiducialLabel->setFocusPolicy(Qt::StrongFocus);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DFiducialWidget::~LAU3DFiducialWidget()
{
    QSettings settings;
    settings.setValue(QString("LAU3DFiducialWidget::lastDirectoryString"), lastDirectoryString);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<QVector3D> LAU3DFiducialWidget::points() const
{
    QList<QVector3D> list;
    for (int n=0; n<pointList.count(); n++){
        list << pointList.at(n).point();
    }
    return(list);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::load(QString filename)
{
    if (filename.isNull()){
        filename = QFileDialog::getOpenFileName(0, QString("Load scan file from disk (*.txt;*.tif)"), lastDirectoryString, QString("*.txt *.tif"));
        if (filename.isNull()){
            return;
        } else {
            lastDirectoryString = QFileInfo(filename).absolutePath();
        }
    }

    if (filename.endsWith(".tif")){
        scanFileString = filename;
        localScan = LAUScan(filename);
        fiducialLabel->setImage(localScan.preview(localScan.size()));
        this->setWindowTitle(scanFileString);
    } else {
        // OPEN TIFF FILE FOR LOADING THE IMAGE
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)){
            filenameString = filename;
            this->setWindowTitle(filenameString);

            QTextStream stream(&file);
            scanFileString = stream.readLine();
            localScan = LAUScan(scanFileString);
            fiducialLabel->setImage(localScan.preview(localScan.size()));
            while (stream.atEnd() == false){
                LAUFiducialPoint point;
                point.loadFrom(&stream);

                // GET THE CURRENT NUMBER OF ITEMS IN TABLE
                int n = table->rowCount();

                // DONT LET THE USER ADD MORE THAN 26 POINTS SINCE WE RUN OUT OF LABELS
                if (n >= 26) break;

                // CREATE A NEW ROW
                table->setRowCount(n+1);
                table->setCurrentCell(n, 0);

                table->setItem(n,0,new QTableWidgetItem(QString("%1").arg(point.x())));
                table->setItem(n,1,new QTableWidgetItem(QString("%1").arg(point.y())));
                table->setItem(n,2,new QTableWidgetItem(QString("%1").arg(point.z())));
                table->setVerticalHeaderItem(n, new QTableWidgetItem(point.label()));

                table->item(n,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                table->item(n,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                table->item(n,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

                // PRESERVE THE NEW POINT IN OUR POINT LIST
                pointList << point;

                // UPDATE THE IMAGE LABEL TO SHOW THE NEW POINT
                fiducialLabel->setPointList(pointList);
                fiducialLabel->setCurrentPoint(n);
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::displayScan(QWidget *parent)
{
    if (localScan.isNull() == false && scanWidget == NULL){
        scanWidget = new LAU3DFiducialGLWidget(localScan.width(), localScan.height(), localScan.color(), localScan.constPointer(), parent);
        scanWidget->setLimits(localScan.minX(), localScan.maxX(), localScan.minY(), localScan.maxY(), localScan.minZ(), localScan.maxZ());
        scanWidget->show();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::save(QString filename)
{
    if (filename.isNull()){
        filename = QFileDialog::getSaveFileName(0, QString("Save scan file to disk (*.txt)"), lastDirectoryString, QString("*.txt"));
        if (filename.isNull()){
            return;
        } else {
            lastDirectoryString = QFileInfo(filename).absolutePath();
        }
    }

    // OPEN TIFF FILE FOR SAVING THE IMAGE
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)){
        filenameString = filename;
        this->setWindowTitle(filenameString);

        QTextStream stream(&file);
        stream << scanFileString << QString("\n");
        for (int n=0; n<pointList.count(); n++){
            pointList.at(n).saveTo(&stream);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::onAddItem(int col, int row)
{
    // GET THE CURRENT NUMBER OF ITEMS IN TABLE
    int n = table->rowCount();

    // DONT LET THE USER ADD MORE THAN 26 POINTS SINCE WE RUN OUT OF LABELS
    if (n >= 26) return;

    // CREATE A NEW ROW
    table->setRowCount(n+1);
    table->setCurrentCell(n, 0);

    // SET THE ROW AND COLUMN COORDINATES
    if (col == -1) col = fiducialLabel->width()/2;
    if (row == -1) row = fiducialLabel->height()/2;

    // GET THE X,Y,Z COORDINATE FOR THE INCOMING PIXEL COORDINATE
    QVector<float> pixel = localScan.pixel(col, row);

    // CREATE A NEW POINT AND ADD TO TABLE
    LAUFiducialPoint point(col, row, pixel[0], pixel[1], pixel[2], QString(QChar(65+n)));

    // ADD NEW POINT TO TABLE LIST
    table->setItem(n,0,new QTableWidgetItem(QString("%1").arg(point.x())));
    table->setItem(n,1,new QTableWidgetItem(QString("%1").arg(point.y())));
    table->setItem(n,2,new QTableWidgetItem(QString("%1").arg(point.z())));
    table->setVerticalHeaderItem(n, new QTableWidgetItem(point.label()));

    // MAKE TABLE LIST ITEM READ ONLY
    table->item(n,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(n,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(n,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    // PRESERVE THE NEW POINT IN OUR POINT LIST
    pointList << point;

    // ADD POINT TO LABEL
    fiducialLabel->setPointList(pointList);
    fiducialLabel->setCurrentPoint(n);

    QList<QVector3D> vectorList;
    for (int n=0; n<pointList.count(); n++){
        vectorList << QVector3D(pointList.at(n).x(), pointList.at(n).y(), pointList.at(n).z());
    }
    if (scanWidget){
        scanWidget->onSetFiducials(vectorList);
    }

    // EMIT THE UPDATE SIGNAL SO THE USER CAN RESPOND AS NEEDED
    emit emitUpdate();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::onDeleteItem()
{
    // GET THE CURRENT TABLE ROW
    int k = table->currentRow();

    // RETURN IF THERE IS NO CURRENT ROW
    if (k == -1) return;

    // GRAB THE LABEL OF THE CURRENT ROW SO WE KNOW WHICH POINT TO DELETE
    QString label = table->verticalHeaderItem(k)->text();

    // FIND THE POINT IN THE LIST AND DELETE IT
    for (int n=0; n<pointList.count(); n++){
        if (pointList.at(n).label() == label){
            pointList.removeAt(n);
            table->removeRow(n);
            break;
        }
    }

    // RESET ALL THE POINT LABELS
    for (int n=0; n<pointList.count(); n++){
        LAUFiducialPoint point = pointList.at(n);
        point.setLabel(QString(QChar(65+n)));
        table->setVerticalHeaderItem(n, new QTableWidgetItem(point.label()));
        pointList.replace(n, point);
    }

    // SELECT THE CURRENT ROW SO USER CAN PRESS DELETE BUTTON OVER AND OVER
    k = qMin(k, table->rowCount()-1);
    if (k >= 0){
        table->setCurrentCell(k,0);
        fiducialLabel->setCurrentPoint(k);
    }
    fiducialLabel->setPointList(pointList);

    QList<QVector3D> vectorList;
    for (int n=0; n<pointList.count(); n++){
        vectorList << QVector3D(pointList.at(n).x(), pointList.at(n).y(), pointList.at(n).z());
    }
    if (scanWidget){
        scanWidget->onSetFiducials(vectorList);
    }

    // EMIT THE UPDATE SIGNAL SO THE USER CAN RESPOND AS NEEDED
    emit emitUpdate();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::onMoveUpItem()
{
    // GET THE CURRENT TABLE ROW
    int k = table->currentRow();

    // RETURN IF THERE IS NO ROOM TO MOVE ITEM UP
    if (k <= 0) return;

    // SWAP THE VALUES IN THE LIST
    LAUFiducialPoint pointA = pointList.at(k);
    LAUFiducialPoint pointB = pointList.at(k-1);

    QString labelA = pointA.label();
    pointA.setLabel(pointB.label());
    pointB.setLabel(labelA);

    pointList.replace(k, pointB);
    pointList.replace(k-1, pointA);
    fiducialLabel->setPointList(pointList);

    // NOW CHANGE THE CELLS IN OUR TABLE TO MATCH
    table->setItem(k,0,new QTableWidgetItem(QString("%1").arg(pointB.x())));
    table->setItem(k,1,new QTableWidgetItem(QString("%1").arg(pointB.y())));
    table->setItem(k,2,new QTableWidgetItem(QString("%1").arg(pointB.z())));
    table->setVerticalHeaderItem(k, new QTableWidgetItem(pointB.label()));
    table->item(k,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    table->setItem(k-1,0,new QTableWidgetItem(QString("%1").arg(pointA.x())));
    table->setItem(k-1,1,new QTableWidgetItem(QString("%1").arg(pointA.y())));
    table->setItem(k-1,2,new QTableWidgetItem(QString("%1").arg(pointA.z())));
    table->setVerticalHeaderItem(k-1, new QTableWidgetItem(pointA.label()));
    table->item(k-1,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k-1,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k-1,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    table->setCurrentCell(k-1,0);
    fiducialLabel->setCurrentPoint(k-1);

    if (scanWidget){
        scanWidget->onSetFiducials(points());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::onMoveDownItem()
{
    // GET THE CURRENT TABLE ROW
    int k = table->currentRow();

    // RETURN IF THERE IS NO ROOM TO MOVE ITEM UP
    if (k < 0 || k == table->rowCount()-1) return;

    // SWAP THE VALUES IN THE LIST
    LAUFiducialPoint pointA = pointList.at(k);
    LAUFiducialPoint pointB = pointList.at(k+1);

    QString labelA = pointA.label();
    pointA.setLabel(pointB.label());
    pointB.setLabel(labelA);

    pointList.replace(k, pointB);
    pointList.replace(k+1, pointA);
    fiducialLabel->setPointList(pointList);

    // NOW CHANGE THE CELLS IN OUR TABLE TO MATCH
    table->setItem(k,0,new QTableWidgetItem(QString("%1").arg(pointB.x())));
    table->setItem(k,1,new QTableWidgetItem(QString("%1").arg(pointB.y())));
    table->setItem(k,2,new QTableWidgetItem(QString("%1").arg(pointB.z())));
    table->setVerticalHeaderItem(k, new QTableWidgetItem(pointB.label()));
    table->item(k,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    table->setItem(k+1,0,new QTableWidgetItem(QString("%1").arg(pointA.x())));
    table->setItem(k+1,1,new QTableWidgetItem(QString("%1").arg(pointA.y())));
    table->setItem(k+1,2,new QTableWidgetItem(QString("%1").arg(pointA.z())));
    table->setVerticalHeaderItem(k+1, new QTableWidgetItem(pointA.label()));
    table->item(k+1,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k+1,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    table->item(k+1,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    table->setCurrentCell(k+1,0);
    fiducialLabel->setCurrentPoint(k+1);

    if (scanWidget){
        scanWidget->onSetFiducials(points());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::onUpdatePoint(QString label, int col, int row)
{
    for (int n=0; n<pointList.count(); n++){
        if (pointList.at(n).label() == label){
            // GET THE XYZ COORDINATE FOR THE INCOMING PIXEL COORDINATE
            QVector<float> pixel = localScan.pixel(col,row);

            // GRAB A LOCAL COPY OF THE  SUBJECT POINT FROM THE POINT LIST
            LAUFiducialPoint point = pointList.at(n);
            point.setRow(row);
            point.setCol(col);
            point.setX(pixel[0]);
            point.setY(pixel[1]);
            point.setZ(pixel[2]);

            // UPDATE THE POINT IN OUR POINT LIST
            pointList.replace(n, point);

            // UPDATE LABELS IN THE TABLE VIEW
            table->setItem(n,0,new QTableWidgetItem(QString("%1").arg(point.x())));
            table->setItem(n,1,new QTableWidgetItem(QString("%1").arg(point.y())));
            table->setItem(n,2,new QTableWidgetItem(QString("%1").arg(point.z())));

            // CREATE A LIST OF 3D POINTS FOR THE OPENGL DISPLAY
            QList<QVector3D> vectorList;
            for (int n=0; n<pointList.count(); n++){
                vectorList << QVector3D(pointList.at(n).x(), pointList.at(n).y(), pointList.at(n).z());
            }
            if (scanWidget){
                scanWidget->onSetFiducials(vectorList);
            }

            // EMIT THE UPDATE SIGNAL SO THE USER CAN RESPOND AS NEEDED
            emit emitUpdate();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialWidget::keyPressEvent(QKeyEvent *event)
{
    // RETURN IF THERE IS NO ROOM TO MOVE ITEM UP
    int k = table->currentRow();

    if (k<0) { return; }

    // SWAP THE VALUES IN THE LIST
    LAUFiducialPoint point = pointList.at(k);
    if (event->key() == Qt::Key_Right){
        point.setCol(qMin((int)point.col()+1, (int)localScan.width()-1));
    } else if (event->key() == Qt::Key_Left){
        point.setCol(qMax((int)point.col()-1, 0));
    } else if (event->key() == Qt::Key_Up){
        point.setRow(qMax((int)point.row()-1, 0));
    } else if (event->key() == Qt::Key_Down){
        point.setRow(qMin((int)point.row()+1, (int)localScan.height()-1));
    }
    pointList.replace(k, point);
    fiducialLabel->updatePoint(point);
    onUpdatePoint(point.label(), point.col(), point.row());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialLabel::updatePoint(LAUFiducialPoint point)
{
    for (int n=0; n<pointList.count(); n++){
        if (point.label() == pointList.at(n).label()){
            pointList.replace(n, point);
            update();
            return;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialLabel::mousePressEvent(QMouseEvent *event)
{
    // CHECK FOR RIGHT BUTTON CLICK
    if (event->button()!=Qt::LeftButton) return;

    buttonDownFlag=false;

    int minDist = 100;
    for (int n=0; n<pointList.count(); n++){
        LAUFiducialPoint point = pointList.at(n);

        int x = point.col() - event->x();
        int y = point.row() - event->y();

        int distance = x*x + y*y;
        if (distance <= minDist){
            minDist = distance;
            buttonDownFlag = true;
            currentActivePointIndex = n;
        }
    }
    if (buttonDownFlag){
        emit emitCurrentPointChanged(currentActivePointIndex);
        update();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (buttonDownFlag && currentActivePointIndex >= 0 && currentActivePointIndex < pointList.count()){
        LAUFiducialPoint point = pointList.at(currentActivePointIndex);
        point.setRow(qMin(qMax(0, event->y()), image.height()-1));
        point.setCol(qMin(qMax(0, event->x()), image.width()-1));
        pointList.replace(currentActivePointIndex, point);

        emit emitPointMoved(point.label(), point.col(), point.row());
    } else {
        return;
    }
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialLabel::mouseReleaseEvent(QMouseEvent*)
{
    buttonDownFlag=false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFiducialLabel::paintEvent(QPaintEvent*)
{
    QPainter painter;

    float xScaleFactor = (float)this->width()/(float)image.width();
    float yScaleFactor = (float)this->height()/(float)image.height();

    yScaleFactor = qMin(xScaleFactor, yScaleFactor);

    QTransform transform;
    transform.scale(yScaleFactor, yScaleFactor);

    painter.begin(this);
    painter.setTransform(transform);
    painter.drawImage(0,0,image);

    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.setPen(QPen(QBrush(Qt::black), 3, Qt::SolidLine));
    for (int n=0; n<pointList.count(); n++){
        if (n != currentActivePointIndex){
            LAUFiducialPoint point = pointList.at(n);
            painter.drawEllipse(point.col()-10, point.row()-10, 20, 20);
            painter.drawText(point.col()-10, point.row()-10, 20, 20, Qt::AlignCenter|Qt::AlignHCenter, point.label());
        }
    }

    if (currentActivePointIndex >= 0 && currentActivePointIndex < pointList.count()){
        LAUFiducialPoint point = pointList.at(currentActivePointIndex);
        painter.setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
        painter.drawEllipse(point.col()-10, point.row()-10, 20, 20);
        painter.drawText(point.col()-10, point.row()-10, 20, 20, Qt::AlignCenter|Qt::AlignHCenter, point.label());
    }

    painter.end();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DFiducialGLWidget::LAU3DFiducialGLWidget(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, unsigned char *buffer, QWidget *parent) : LAU3DScanGLWidget(cols, rows, color, buffer, parent), fiducialRadius(0.30f), fiducialDragMode(false), enableFiducialFlag(true), currentActivePointIndex(-1)
{
    // MAKE SURE WE CAN SEND VECTOR3D LISTS AS SIGNALS
    qRegisterMetaType< QList<QVector3D> >("QList<QVector3D>");

    for (unsigned int n=0; n < 26; n++){
        fiducialTextures[n] = NULL;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAU3DFiducialGLWidget::~LAU3DFiducialGLWidget()
{
    if (wasInitialized()){
        makeCurrent();
        for (unsigned int n=0; n < 26; n++){
            if (fiducialTextures[n]){
                delete fiducialTextures[n];
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::updateFiducialProjectionMatrix()
{
    // INITIALIZE THE PROJECTION MATRIX TO IDENTITY
    float fov = qMin(120.0f, qMax(180.0f/3.1415f * verticalFieldOfView, 0.5f));
    float aspectRatio = (float)width()/(float)height();

    fiducialProjection.setToIdentity();
    fiducialProjection.perspective(fov, aspectRatio, 1.0f, 10000.0f);
    fiducialProjection.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, zMax), QVector3D(0.0f, 1.0f, 0.0f));

    // CALCULATE THE FIDUCIAL RADIUS AS A FUNCTION OF THE DISPLAY DPI
    fiducialRadius = 17.5f/(float)width();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::mousePressEvent(QMouseEvent *event)
{
    // TELL THE WORLD THAT WE HAVE BEEN ACTIVATED
    emit emitActivated();

    // SET THE FIDUCIAL DRAGGING FLAGS TO FALSE UNTIL WE KNOW BETTER
    fiducialDragMode = false;

    if (enableFiducialFlag && event->button() == Qt::LeftButton && fiducialList.count() > 0){
        // SEE IF WE ARE IN CLOSE PROXIMITY TO A FIDUCIAL
        float x = 2.0f*(float)event->pos().x()/(float)this->width() - 1.0f;
        float y = 2.0f*(float)event->pos().y()/(float)this->height() - 1.0f;

        // SET THE THRESHOLD FOR HOW CLOSE A MOUSE CLICK NEEDS TO BE TO BE CONSIDERED A FIDUCIAL CLICK
        float tolerance = fiducialRadius*width()/zoomFactor;

        // STORE THE CLOSEST FIDUCIAL
        QVector3D closestFiducial(1e9, 1e9, -1e9);

        for (int n=0; n<fiducialList.count(); n++){
            // CALCULATE THE WINDOW COORDINATE OF THE CURRENT FIDUCIAL
            QVector3D fiducial = fiducialList.at(n);
            QVector4D coordinate = projection * QVector4D(fiducial.x(), fiducial.y(), fiducial.z(), 1.0f); coordinate = coordinate/coordinate.w();

            // CALCULATE THE DISTANCE FROM THE FIDUCIAL TO THE EVENT COORDINATE IN PIXELS
            QVector2D position((coordinate.x()-x)/2.0f*(float)width(), (coordinate.y()+y)/2.0f*(float)height());

            // SEE IF THIS FIDUCIAL IS WITHIN A CLOSE PROXIMITY OF THE MOUSE CLICK
            if (position.length() < tolerance){
                if (fiducial.z() > closestFiducial.z()){
                    fiducialDragMode = true;
                    currentActivePointIndex = n;
                    closestFiducial = fiducial;
                }
            }
        }

        // CHECK TO SEE IF WE SHOULD ENABLE FIDUCIAL TRACKING
        if (fiducialDragMode){
            // AND WE NEED TO GRAB A COPY OF THE MOUSE BUFFER TO DRAGGING THE FIDUCIAL
            screenMap = grabMouseBuffer(LAU3DScanGLWidget::MouseModeXYZ);
            colorMap = grabMouseBuffer(LAU3DScanGLWidget::MouseModeRGB);

            // UPDATE THE SCREEN SO THAT WE AT LEAST CHANGE THE COLOR OF THE CURRENT FIDUCIAL
            update();
        } else {
            // LET THE UNDERLYING CLASS HANDLE THE EVENT
            LAU3DScanGLWidget::mousePressEvent(event);
        }
    } else {
        // LET THE UNDERLYING CLASS HANDLE THE EVENT
        LAU3DScanGLWidget::mousePressEvent(event);

        // UPDATE THE FIDUCIAL PROJECTION MATRIX IN CASE THE PROJECTION MATRIX CHANGED
        updateFiducialProjectionMatrix();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (fiducialDragMode){
        fiducialDragMode = false;

        // EMIT THE FIDUCIALS/COLORS TO THE USER
        emitFiducialsChanged(fiducialList);
        emitFiducialsChanged(fiducialList, colorsList);
    } else {
        // LET THE UNDERLYING CLASS HANDLE THE EVENT
        LAU3DScanGLWidget::mouseReleaseEvent(event);

        // UPDATE THE FIDUCIAL PROJECTION MATRIX IN CASE THE PROJECTION MATRIX CHANGED
        updateFiducialProjectionMatrix();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (enableFiducialFlag && event->button() == Qt::LeftButton){
        // WE NEED TO GRAB A COPY OF THE MOUSE BUFFER TO POSITION THE FIDUCIAL
        screenMap = grabMouseBuffer(LAU3DScanGLWidget::MouseModeXYZ);
        colorMap = grabMouseBuffer(LAU3DScanGLWidget::MouseModeRGB);

        // CONVERT THE WIDGET COORDINATE TO A SCREEN MAP COORDINATE
        int row = (1.0f - (float)event->pos().y()/(float)height()) * (int)screenMap.height();
        int col = (float)event->pos().x()/(float)width() * (int)screenMap.width();

        if (row >= 0 && row<(int)screenMap.height() && col >= 0 && col<(int)screenMap.width()){
            // GRAB THE XYZW-PIXEL FOR THE CURRENT MOUSE POSITION
            float *pixel = &((float*)screenMap.constScanLine(row))[4*col];
            float *color = &((float*)colorMap.constScanLine(row))[4*col];

            // MAKE SURE THAT THE W-COORDINATE IS NOT EQUAL TO 0.0F
            if (pixel[3] > 0.5f){
                // APPEND THE FIDUCIAL LIST WITH THE CURRENT MOUSE POSITION
                fiducialList.append(QVector3D(pixel[0], pixel[1], pixel[2]));
                colorsList.append(QVector3D(color[0], color[1], color[2]));
                currentActivePointIndex = fiducialList.count()-1;

                // EMIT THE FIDUCIALS/COLORS TO THE USER
                emit emitFiducialsChanged(fiducialList, colorsList);
                emit emitFiducialsChanged(fiducialList);

                // DRAW THE NEW FIDUCIAL ON SCREEN
                update();
            } else {
                // IF THE USER DOUBLE CLICKS AN AREA WITH NO SCAN DATA, THEN CALL
                // THE UNDERLYING CLASS TO HANDLE THE DOUBLE CLICK
                LAU3DScanGLWidget::mouseDoubleClickEvent(event);

                // UPDATE THE FIDUCIAL PROJECTION MATRIX IN CASE THE PROJECTION MATRIX CHANGED
                updateFiducialProjectionMatrix();
            }
        }
    } else {
        // LET THE UNDERLYING CLASS HANDLE THE EVENT
        LAU3DScanGLWidget::mouseDoubleClickEvent(event);

        // UPDATE THE FIDUCIAL PROJECTION MATRIX IN CASE THE PROJECTION MATRIX CHANGED
        updateFiducialProjectionMatrix();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (fiducialDragMode && currentActivePointIndex >= 0){
        // CONVERT THE WIDGET COORDINATE TO A SCREEN MAP COORDINATE
        int row = (1.0f - (float)event->pos().y()/(float)height()) * (int)screenMap.height();
        int col = (float)event->pos().x()/(float)width() * (int)screenMap.width();

        if (row >= 0 && row<(int)screenMap.height() && col >= 0 && col<(int)screenMap.width()){
            // GRAB THE XYZW-PIXEL FOR THE CURRENT MOUSE POSITION
            float *pixel = &((float*)screenMap.constScanLine(row))[4*col];
            float *color = &((float*)colorMap.constScanLine(row))[4*col];

            // MAKE SURE THAT THE W-COORDINATE IS NOT EQUAL TO 0.0F
            if (pixel[3] > 0.5f){
                // REPLACE THE FIDUCIAL WITH THE CURRENT MOUSE POSITION
                QVector3D fiducial = QVector3D(pixel[0], pixel[1], pixel[2]);
                QVector3D colors = QVector3D(color[0], color[1], color[2]);
                fiducialList.replace(currentActivePointIndex, fiducial);
                colorsList.replace(currentActivePointIndex, colors);

                // EMIT THE ALTERED FIDUCIALS/COLORS
                emit emitFiducialsChanged(fiducial, currentActivePointIndex);
                emit emitFiducialsChanged(fiducial, currentActivePointIndex, colors);

                // UPDATE THE DISPLAY
                update();
            }
        }
    } else {
        // LET THE UNDERLYING CLASS HANDLE THE EVENT
        LAU3DScanGLWidget::mouseMoveEvent(event);

        // UPDATE THE FIDUCIAL PROJECTION MATRIX IN CASE THE PROJECTION MATRIX CHANGED
        updateFiducialProjectionMatrix();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::keyPressEvent(QKeyEvent *event)
{
    // IF THERE IS NO CURRENTLY ACTIVE POINTS, THEN QUIT
    if (enableFiducialFlag && currentActivePointIndex >= 0) {
        // CHECK FOR AN UP OR DOWN ARROW TO CHANGE THE CURRENT FIDUCIAL
        // CHECK FOR DELETE TO REMOVE A FIDUCIAL FROM THE LIST
        if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Up){
            currentActivePointIndex = (currentActivePointIndex+1)%fiducialList.count();

            // EMIT THE FIDUCIALS/COLORS TO THE USER
            emit emitFiducialsChanged(fiducialList.at(currentActivePointIndex), currentActivePointIndex);
            emit emitFiducialsChanged(fiducialList.at(currentActivePointIndex), currentActivePointIndex, colorsList.at(currentActivePointIndex));
        } else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Down){
            currentActivePointIndex = (currentActivePointIndex+fiducialList.count()-1)%fiducialList.count();

            // EMIT THE FIDUCIALS/COLORS TO THE USER
            emit emitFiducialsChanged(fiducialList.at(currentActivePointIndex), currentActivePointIndex);
            emit emitFiducialsChanged(fiducialList.at(currentActivePointIndex), currentActivePointIndex, colorsList.at(currentActivePointIndex));
        } else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace){
            fiducialList.removeAt(currentActivePointIndex);
            currentActivePointIndex = qMin(currentActivePointIndex, fiducialList.count()-1);

            // EMIT THE FIDUCIALS/COLORS TO THE USER
            emitFiducialsChanged(fiducialList, colorsList);
            emitFiducialsChanged(fiducialList);
        }
        update();
    } else {
        // LET THE UNDERLYING CLASS HANDLE THE EVENT
        LAU3DScanGLWidget::keyPressEvent(event);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::wheelEvent(QWheelEvent *event)
{
    // IGNORE WHEEL EVENTS IF WE ARE IN DRAG MODE
    if (fiducialDragMode == false){
        // LET THE UNDERLYING CLASS HANDLE THE EVENT
        LAU3DScanGLWidget::wheelEvent(event);

        // UPDATE THE FIDUCIAL PROJECTION MATRIX IN CASE THE PROJECTION MATRIX CHANGED
        updateFiducialProjectionMatrix();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::initializeGL()
{
    LAU3DScanGLWidget::initializeGL();

    // CREATE THE VERTEX BUFFER TO HOLD THE XYZ COORDINATES PLUS THE
    // TEXTURE COORDINATES (5 FLOATS PER VERTEX, 4 VERTICES PER SIDE, 6 SIDES)
    fiducialVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    fiducialVertexBuffer.create();
    fiducialVertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if (fiducialVertexBuffer.bind()){
        fiducialVertexBuffer.allocate(120*sizeof(float));
        float *vertices = (float*)fiducialVertexBuffer.map(QOpenGLBuffer::WriteOnly);
        int i = 0;
        if (vertices){
            // TOP/BOTTOM SURFACES
            vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 1; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 1;
            vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = 0; vertices[i++] = 1;

            vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 1;
            vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = 1; vertices[i++] = 1;
            vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 0;
            vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = 0; vertices[i++] = 0;

            // LEFT/RIGHT SURFACES
            vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0;
            vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 1;
            vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 1;
            vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 0;

            vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 1;
            vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 1;
            vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 0;

            // FRONT/BACK SURFACES
            vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 1; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = 1; vertices[i++] = 1;
            vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 1;

            vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = 0; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 0;
            vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = 1; vertices[i++] = 1;
            vertices[i++] = -1; vertices[i++] = +1; vertices[i++] = +1; vertices[i++] = 0; vertices[i++] = 1;
            fiducialVertexBuffer.unmap();
        } else {
            qDebug() << QString("fiducialVertexBuffer buffer mapped from GPU.");
        }
        fiducialVertexBuffer.release();
    }

    // CREATE THE FIDUCIAL INDEX BUFFER
    fiducialIndiceBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    fiducialIndiceBuffer.create();
    fiducialIndiceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (fiducialIndiceBuffer.bind()){
        fiducialIndiceBuffer.allocate(6*6*sizeof(unsigned int));
        unsigned int *indices = (unsigned int*)fiducialIndiceBuffer.map(QOpenGLBuffer::WriteOnly);
        int i=0;
        if (indices){
            for (unsigned int j=0; j<6; j++){
                indices[i++] = 4*j+0; indices[i++] = 4*j+1; indices[i++] = 4*j+2;
                indices[i++] = 4*j+0; indices[i++] = 4*j+2; indices[i++] = 4*j+3;
            }
            fiducialIndiceBuffer.unmap();
        } else {
            qDebug() << QString("No fiducialIndiceBuffer mapped from GPU.");
        }
        fiducialIndiceBuffer.release();
    }

    // CREATE TEXTURES FOR FIDUCIAL CUBES
    for (unsigned int n=0; n < 26; n++){
        fiducialTextures[n] = new QOpenGLTexture(QImage(QString(":/Fiducials/letter%1.tif").arg(char(65+n))).mirrored());
        fiducialTextures[n]->setWrapMode(QOpenGLTexture::ClampToBorder);
        fiducialTextures[n]->setMinificationFilter(QOpenGLTexture::Nearest);
        fiducialTextures[n]->setMagnificationFilter(QOpenGLTexture::Nearest);
    }

    // COMPILE THE SHADER LANGUAGE PROGRAM FOR DRAWING FIDUCIALS
    setlocale(LC_NUMERIC, "C");
    if (!fiducialProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/MISC/drawFiducials.vert")) close();
    if (!fiducialProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/MISC/drawFiducials.frag")) close();
    if (!fiducialProgram.link()) close();
    setlocale(LC_ALL, "");

    // MAKE SURE TO INITIALIZE THE FIDUCIAL PROJECTION MATRIX
    updateFiducialProjectionMatrix();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAU3DFiducialGLWidget::paintGL()
{
    // CALLS THE BASECLASSES PAINT METHOD IN ORDER TO DISPLAY THE LAU SCAN SURFACES
    LAU3DScanGLWidget::paintGL();

    // BIND THE FIDUCIAL PROGRAM SO THAT WE CAN NOW DRAW THE FIDUCIALS FOR THE SCAN
    if (enableFiducialFlag && fiducialList.count() > 0 && fiducialProgram.bind()){
        // BIND THE FIDUCIAL VERTEX BUFFER
        if (fiducialVertexBuffer.bind()){
            // BIND THE FIDUCIAL INDICE BUFFER
            if (fiducialIndiceBuffer.bind()){
                // ENABLE THE ATTRIBUTE ARRAY
                glVertexAttribPointer(fiducialProgram.attributeLocation("qt_vertexA"), 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
                fiducialProgram.enableAttributeArray("qt_vertexA");

                // ENABLE THE ATTRIBUTE ARRAY
                glVertexAttribPointer(fiducialProgram.attributeLocation("qt_vertexB"), 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (const void*)(3*sizeof(float)));
                fiducialProgram.enableAttributeArray("qt_vertexB");

                // DERIVE THE PROJECTION MATRIX
                fiducialProgram.setUniformValue("qt_projection", projection);

                // ITERATE THROUGH THE FIDUCIAL, LIST DRAWING EACH ONE AT A TIME
                for (int n=0; n<fiducialList.count() && n<26; n++){
                    // MAKE SURE WE AREN'T TRYING TO DISPLAY A NAN POINT
                    if (qIsNaN(fiducialList.at(n).x())) continue;

                    // BIND THE FIDUCIAL TEXTURE
                    glActiveTexture(GL_TEXTURE1 + n);
                    fiducialTextures[n]->bind();

                    if (n == currentActivePointIndex){
                        // SET THE COLOR INVERSE TRANSFORM MATRIX
                        fiducialProgram.setUniformValue("qt_colorMat", QMatrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f,-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
                    } else {
                        // SET THE COLOR IDENTITY TRANSFORM MATRIX
                        fiducialProgram.setUniformValue("qt_colorMat", QMatrix4x4());
                    }

                    // PASS THE HANDLE TO OUR FRAME BUFFER OBJECT'S TEXTURE
                    fiducialProgram.setUniformValue("qt_texture", n+1);

                    // CALCULATE THE POSITION OF THE SCENE FOCAL POINT
                    QVector4D pointA = fiducialProjection * QVector4D(fiducialList.at(n).x(), fiducialList.at(n).y(), fiducialList.at(n).z(), 1.0f); pointA = pointA/pointA.w();
                    QVector4D pointB = fiducialProjection.inverted() * QVector4D(fiducialRadius, 0.0f, pointA.z(), pointA.w());                      pointB = pointB/pointB.w();

                    // SPECIFY THE SIZE OF THE FRAME BUFFER OBJECT'S TEXTURE
                    fiducialProgram.setUniformValue("qt_radius", (float)qFabs(pointB.x()));

                    // SET FIDUCIAL COORDINATE
                    fiducialProgram.setUniformValue("qt_fiducial", fiducialList.at(n));

                    // DRAW THE ELEMENTS
                    glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

                    // RELEASE THE TEXTURE BUFFER
                    fiducialTextures[n]->release();
                }
                // RELEASE THE INDICE BUFFER
                fiducialIndiceBuffer.release();
            }
            // RELEASE THE VERTEX BUFFER
            fiducialVertexBuffer.release();
        }
        // RELEASE THE PROGRAM
        fiducialProgram.release();
    }
}
