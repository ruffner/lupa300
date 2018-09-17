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

#ifndef LAU3DFIDUCIALGLWIDGET_H
#define LAU3DFIDUCIALGLWIDGET_H

#include <QPen>
#include <QList>
#include <QBrush>
#include <QImage>
#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QVector3D>
#include <QPainter>
#include <QFileInfo>
#include <QKeyEvent>
#include <QSettings>
#include <QTransform>
#include <QTabWidget>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QHeaderView>
#include <QTableWidget>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <QDialogButtonBox>
#include <QAbstractItemView>

#include "lauscan.h"
#include "lau3dscanglwidget.h"

using namespace LAU3DVideoParameters;

class LAUFiducialLabel;
class LAUFiducialPoint;
class LAU3DFiducialWidget;
class LAUFiducialDialog;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DFiducialGLWidget : public LAU3DScanGLWidget
{
    Q_OBJECT

public:
    LAU3DFiducialGLWidget(unsigned int cols, unsigned int rows, LAUVideoPlaybackColor color, unsigned char *buffer = NULL, QWidget *parent = 0);
    ~LAU3DFiducialGLWidget();

    QList<QVector3D> fiducials() const { return(fiducialList); }

public slots:
    void onEnableFiducials(bool state) { enableFiducialFlag = state; update(); }
    void onSetFiducials(QList<QVector3D> fiducials) { fiducialList = fiducials; updateFiducialProjectionMatrix(); update(); }
    void onKeyPressEvent(QKeyEvent *event) { keyPressEvent(event); }

protected:
    void updateFiducialProjectionMatrix();

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

    void resizeGL(int w, int h) { LAU3DScanGLWidget::resizeGL(w,h); updateFiducialProjectionMatrix(); }
    void initializeGL();
    void paintGL();

    QOpenGLBuffer fiducialVertexBuffer, fiducialIndiceBuffer;
    QOpenGLShaderProgram fiducialProgram;
    QOpenGLTexture *fiducialTextures[26];
    QList<QVector3D> fiducialList;
    QList<QVector3D> colorsList;

    float fiducialRadius;
    bool fiducialDragMode;
    bool enableFiducialFlag;
    QMatrix4x4 fiducialProjection;
    int maxNumberFiducials;
    int currentActivePointIndex;
    LAUMemoryObject screenMap;
    LAUMemoryObject colorMap;

signals:
    void emitFiducialsChanged(QVector3D, int);
    void emitFiducialsChanged(QList<QVector3D>);
    void emitFiducialsChanged(QVector3D, int, QVector3D);
    void emitFiducialsChanged(QList<QVector3D>, QList<QVector3D>);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFiducialPoint
{
public:
    LAUFiducialPoint(int ci=0, int ri=0, float xi=0.0f, float yi=0.0f, float zi=0.0f, QString str=QString()) : c(ci), r(ri), xp(xi), yp(yi), zp(zi), string(str) { ; }
    LAUFiducialPoint(const LAUFiducialPoint &other) { r = other.r; c = other.c; xp = other.xp; yp = other.yp; zp = other.zp; string = other.string; }
    LAUFiducialPoint& operator = ( const LAUFiducialPoint& other ) { if (this != &other) { r = other.r; c = other.c; xp = other.xp; yp = other.yp; zp = other.zp; string = other.string; } return(*this);}

    int row() const { return(r); }
    int col() const { return(c); }

    float x() const { return(xp); }
    float y() const { return(yp); }
    float z() const { return(zp); }

    bool isValid() const { return((qIsNaN(xp) | qIsNaN(yp) | qIsNaN(zp)) == false); }
    QVector3D point() const { return(QVector3D(xp,yp,zp)); }

    QString label() const { return(string); }

    void setRow(int rp) { r = rp; }
    void setCol(int cp) { c = cp; }

    void setX(float xi) { xp = xi; }
    void setY(float yi) { yp = yi; }
    void setZ(float zi) { zp = zi; }

    void setLabel(QString str) { string = str; }

    void saveTo(QTextStream *stream) const
    {
        *stream << string << QString(",") << c << QString(",") << r << QString(",") << xp << QString(",") << yp << QString(",") << zp << QString("\n");
    }

    void loadFrom(QTextStream *stream)
    {
        QStringList strings = stream->readLine().split(",");
        string = strings.takeFirst();
        c = strings.takeFirst().toInt();
        r = strings.takeFirst().toInt();
        xp = strings.takeFirst().toFloat();
        yp = strings.takeFirst().toFloat();
        zp = strings.takeFirst().toFloat();
    }

private:
    int c,r;
    float xp,yp,zp;
    QString string;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAU3DFiducialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAU3DFiducialWidget(LAUScan scan = LAUScan(), QWidget *parent = 0);
    ~LAU3DFiducialWidget();

    void save(QString filename = QString());
    void load(QString filename = QString());

    void displayScan(QWidget *parent = NULL);

    QList<QVector3D> points() const;
    QList<LAUFiducialPoint> fiducials() const { return(pointList); }
    LAUScan scan() const { return(localScan); }

    static QString lastDirectoryString;

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTableWidget *table;
    QString filenameString;
    QString scanFileString;
    QToolButton *newButton;
    QToolButton *delButton;
    QToolButton *upButton;
    QToolButton *dwnButton;
    LAUFiducialLabel *fiducialLabel;
    LAU3DFiducialGLWidget *scanWidget;
    LAUScan localScan;

    QList<LAUFiducialPoint> pointList;

private slots:
    void onAddItem(int col = -1, int row = -1);
    void onDeleteItem();
    void onMoveUpItem();
    void onMoveDownItem();

    void onUpdatePoint(QString label, int col, int row);

signals:
    void emitUpdate();
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFiducialLabel : public QLabel
{
    Q_OBJECT

public:
    explicit LAUFiducialLabel(QImage img = QImage(), QWidget *parent = 0) : QLabel(parent)
    {
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setImage(img);
    }
    ~LAUFiducialLabel() { ; }

    void setImage(QImage in)
    {
        image = in;
        this->setFixedSize(image.width(), image.height());
    }

    int height() const { return(image.height()); }
    int width() const { return(image.width()); }
    QRgb pixel(int col, int row) const { return(image.pixel(col, row)); }

public slots:
    void updatePoint(LAUFiducialPoint point);
    void setCurrentPoint(int currentRow, int currentColumn = 0, int previousRow = 0, int previousColumn = 0) { Q_UNUSED(currentColumn); Q_UNUSED(previousRow); Q_UNUSED(previousColumn); currentActivePointIndex = currentRow; update(); }
    void setPointList(QList<LAUFiducialPoint> list) { pointList = list; update(); }

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) { emit emitDoubleClick(event->pos().x(), event->pos().y()); }
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private:
    QImage image;
    bool buttonDownFlag;
    int currentActivePointIndex;
    QList<LAUFiducialPoint> pointList;

signals:
    void emitDoubleClick(int col, int row);
    void emitPointMoved(QString label, int col, int row);
    void emitCurrentPointChanged(int);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFiducialDialog : public QDialog
{
    Q_OBJECT

public:
    LAUFiducialDialog(LAUScan scan, QWidget *parent = 0) : QDialog(parent)
    {
        widget = new LAU3DFiducialWidget(scan, this);
        this->setWindowTitle(widget->windowTitle());

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6,6,6,6);
        this->layout()->setSpacing(6);
        this->layout()->addWidget(widget);
        ((QVBoxLayout*)this->layout())->addStretch();
        this->layout()->addWidget(buttonBox);
    }
    ~LAUFiducialDialog() { ; }

protected:
    void accept()
    {
        if (QMessageBox::warning(this, QString("Scoliosis Widget"), QString("Save file to disk?"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes){
            widget->save();
        }
        QDialog::accept();
    }
    void reject() { QDialog::reject(); }

private:
    LAU3DFiducialWidget *widget;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFiducialTool : public QWidget
{
    Q_OBJECT

public:
    explicit LAUFiducialTool(QWidget *parent = NULL);

public slots:
    void onFiducialsChanged(QVector3D point, int index, QVector3D color);
    void onFiducialsChanged(QList<QVector3D> points, QList<QVector3D> colors);

private:
    QList<QVector3D> fiducialList;
    QTableWidget *table;
};


#endif // LAU3DFIDUCIALGLWIDGET_H
