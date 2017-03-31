#include "ppwindow.h"
#include "ui_ppwindow.h"
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QColorDialog>
#include <windows.h>

using namespace cv;

PPWindow::PPWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PPWindow)
{
    ui->setupUi(this);
    ui->verNumber->setText("Ver. 0.0.0.1");
    setObject = false;
    setCurrentBackground(ui->backSelect->currentText());
    currColorClust = QColor(0, 0, 0);
    currColorClrz = QColor(255, 255, 255);
    ui->invertSel->setDisabled(true);
    ui->colorizeBtn->setDisabled(true);
    ui->changeColorBtn->setDisabled(true);
    ui->saveBtn->setDisabled(true);
    ui->warningLabel->hide();
    connect(ui->backSelect, &QComboBox::currentTextChanged, this, &PPWindow::setCurrentBackground);
    connect(ui->loadImageBut, &QPushButton::clicked, this, &PPWindow::loadImageWithObj);
    connect(ui->invertSel, &QCheckBox::stateChanged, this, &PPWindow::invertSelection);
    connect(ui->changeColorBtn, &QPushButton::clicked, this, &PPWindow::changeColorClust);
    connect(ui->colorizeBtn,  &QPushButton::clicked, this, &PPWindow::colorizeSpaces);
    connect(ui->saveBtn,  &QPushButton::clicked, this, &PPWindow::saveImage);
}

PPWindow::~PPWindow()
{
    delete ui;
}

void PPWindow::setCurrentBackground(QString name){
    if (name == "Котики"){
        currBack = loadFromQrc(":/backgrounds/kitties.jpg");
    } else
        if (name == "Точки"){
            currBack = loadFromQrc(":/backgrounds/points.jpg");
        } else
            if (name == "Лучи"){
                currBack = loadFromQrc(":/backgrounds/rays.jpg");
            } else
                if (name == "Небо"){
                    currBack = loadFromQrc(":/backgrounds/sky.jpg");
                } else
                    if (name == "Космос"){
                        currBack = loadFromQrc(":/backgrounds/space.jpg");
                    } else
                        if (name == "Клубничка"){
                            currBack = loadFromQrc(":/backgrounds/strawberry.jpg");
                        } else
                            if (name == "Углы"){
                                currBack = loadFromQrc(":/backgrounds/angles.jpg");
                            } else
                                if (name == "Воронка"){
                                    currBack = loadFromQrc(":/backgrounds/vortex.jpg");
                                }
    changeCurrentImage();
}

void PPWindow::changeCurrentImage(){
    if (!setObject){
        currShowMat = currBack.clone();
    } else {
        currShowMat = currBack.clone();
        for (int y = 0; y < currShowMat.rows; ++y) {
            for (int x = 0; x < currShowMat.cols; ++x) {
                if ((colorizeForOut.at<Vec3b>(y, x)[0] != 0) ||
                        (colorizeForOut.at<Vec3b>(y, x)[1] != 0) ||
                        (colorizeForOut.at<Vec3b>(y, x)[2] != 5)){
                    currShowMat.at<Vec3b>(y, x)[0] = colorizeForOut.at<Vec3b>(y, x)[0];
                    currShowMat.at<Vec3b>(y, x)[1] = colorizeForOut.at<Vec3b>(y, x)[1];
                    currShowMat.at<Vec3b>(y, x)[2] = colorizeForOut.at<Vec3b>(y, x)[2];
                }
            }
        }
        QRgb tmpColor = currColorClust.rgb();
        for (int y = 0; y < currShowMat.rows; ++y) {
            for (int x = 0; x < currShowMat.cols; ++x) {
                if (objBinMask.at<uchar>(y, x) == 0){
                    currShowMat.at<Vec3b>(y, x)[0] = qBlue(tmpColor);
                    currShowMat.at<Vec3b>(y, x)[1] = qGreen(tmpColor);
                    currShowMat.at<Vec3b>(y, x)[2] = qRed(tmpColor);
                }
            }
        }
    }
    cv::resize(currShowMat, currShowMat, Size(300, 300));
    Mat3b tmp = currShowMat;

    currShowQImg = QImage(tmp.cols, tmp.rows, QImage::Format_ARGB32);
    for (int y = 0; y < tmp.rows; ++y) {
        const cv::Vec3b *srcrow = tmp[y];
        QRgb *destrow = (QRgb*)currShowQImg.scanLine(y);
        for (int x = 0; x < tmp.cols; ++x) {
            destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
        }
    }

    currShowQPixmap = QPixmap::fromImage(currShowQImg);
    ui->currImage->setPixmap(currShowQPixmap);
}

void PPWindow::invertSelection(){
    threshold(objBinMask, objBinMask, 128, 255, THRESH_BINARY_INV);
    cvtColor(objBinMask, colorizeBinMask, COLOR_GRAY2BGR);
    changeCurrentImage();
}

void PPWindow::loadImageWithObj(){
    QString imagename = QFileDialog::getOpenFileName(0, "Загрузка картинки",
                                                     QDir::current().absolutePath(),
                                                     "*.jpg")
            .toLocal8Bit();

    if (!(imagename.isEmpty()) ){
        bool widthStr = false;
        double koeffStr = 0;
        startObjImg = imread(imagename.toStdString().c_str());
        if (startObjImg.rows > startObjImg.cols){
            koeffStr = (double)startObjImg.rows / 700.0;
            widthStr = true;
            cv::resize(startObjImg, startObjImg, Size(int(double(startObjImg.cols)/koeffStr), 700));
        }else{
            koeffStr = (double)startObjImg.cols / 700.0;
            cv::resize(startObjImg, startObjImg, Size(700, int(double(startObjImg.rows)/koeffStr)));
        }
        setObject = true;

        Mat tmpGrayImg = startObjImg.clone();
        cvtColor(startObjImg, tmpGrayImg, COLOR_BGR2GRAY);

        int maxBright = 0;
        int rows = tmpGrayImg.rows, cols = tmpGrayImg.cols;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (tmpGrayImg.at<unsigned char>(i, j) > maxBright)
                    maxBright = tmpGrayImg.at<unsigned char>(i, j);

        std::vector<unsigned long> hist;
        for (int i = 0; i < 256; ++i) {
            hist.push_back(0);
        }
        for (int i = 0; i < tmpGrayImg.rows; ++i) {
            for (int j = 0; j < tmpGrayImg.cols; ++j) {
                hist.at(tmpGrayImg.at<unsigned char>(i, j))++;
            }
        }
        int ft_max = 1;
        double q1 = 0;
        double q2 = 0;
        double mu1 = 0,	mu2 = 0, mu = 0;
        double variance1 = 0, variance2 = 0, varianceB = 0, variance_max = 0;

        for (int ft = 1; ft <= maxBright; ++ft)
        {
            variance1 = 0;
            variance2 = 0;
            varianceB = 0;
            mu1 = 0;
            q1 = 0;
            q2 = 0;
            mu2 = 0;

            for (int i = 0; i < ft; ++i)
                q1 += hist[i];
            for (int i = ft; i <= maxBright; ++i)
                q2 += hist[i];
            for (int i = 0; i < ft; ++i)
                mu1 += (double)(i)* hist[i] / q1;
            for (int i = ft; i <= maxBright; ++i)
                mu2 += (double)i * hist[i] / q2;
            mu = q1 * mu1 + q2 * mu2;
            for (int i = 0; i < ft; ++i)
                variance1 += ((double)i - mu1) * ((double)i - mu1) * hist[i];
            variance1 /= q1;
            for (int i = ft; i <= maxBright; ++i)
                variance2 += ((double)i - mu2) * ((double)i - mu2) * hist[i];
            variance2 /= q2;
            varianceB = q1 * (mu1 - mu) * (mu1 - mu) + q2 * (mu2 - mu) * (mu2 - mu);
            if (varianceB > variance_max)
            {
                variance_max = varianceB;
                ft_max = ft;
            }
        }

        Mat tmpObjBinMask = Mat(startObjImg.rows, startObjImg.cols, CV_8UC1, Scalar(0));

        for (int i = 0; i < tmpGrayImg.rows; ++i)
            for (int j = 0; j < tmpGrayImg.cols; ++j)
                if (tmpGrayImg.at<unsigned char>(i, j) < ft_max)
                    tmpObjBinMask.at<unsigned char>(i, j) = 0;
                else
                    tmpObjBinMask.at<unsigned char>(i, j) = 255;

        objBinMask = Mat(700, 700, CV_8UC1, Scalar(255));
        if (widthStr){
            int shift = (700 - startObjImg.cols) / 2;
            for (int i = 0; i < tmpGrayImg.rows; ++i)
                for (int j = 0; j < tmpGrayImg.cols; ++j)
                    objBinMask.at<unsigned char>(i, shift + j) = tmpObjBinMask.at<unsigned char>(i, j);
        }else{
            int shift = 700 - startObjImg.rows;
            for (int i = 0; i < tmpGrayImg.rows; ++i)
                for (int j = 0; j < tmpGrayImg.cols; ++j)
                    objBinMask.at<unsigned char>(shift + i, j) = tmpObjBinMask.at<unsigned char>(i, j);
        }

        colorizeBinMask = objBinMask.clone();
        cvtColor(objBinMask, colorizeBinMask, COLOR_GRAY2BGR);
        colorizeForOut = Mat(objBinMask.rows, objBinMask.cols, CV_8UC3, Scalar(0, 0, 5));

        changeCurrentImage();
        ui->invertSel->setDisabled(false);
        ui->changeColorBtn->setDisabled(false);
        ui->colorizeBtn->setDisabled(false);
        ui->saveBtn->setDisabled(false);
    }
}

Mat PPWindow::loadFromQrc(QString qrc, int flag)
{
    QFile file(qrc);
    Mat m;
    if(file.open(QIODevice::ReadOnly))
    {
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*)buf.data(), sz);
        m = imdecode(buf, flag);
    }
    return m;
}

void PPWindow::changeColorClust(){
    QColor tmpColor = QColorDialog::getColor(currColorClust, this, "Выбор цвета контура");
    if (tmpColor.isValid()){
        currColorClust = tmpColor;
        changeCurrentImage();
    }
}

void PPWindow::colorizeSpaces(){
    bool keyHandled = false;
    ui->warningLabel->show();
    ui->backSelect->setDisabled(true);
    ui->invertSel->setDisabled(true);
    ui->changeColorBtn->setDisabled(true);
    ui->colorizeBtn->setDisabled(true);
    ui->loadImageBut->setDisabled(true);
    ui->saveBtn->setDisabled(true);
    QColor tmpColor = QColorDialog::getColor(currColorClrz, this, "Выбор цвета контура");
    if (tmpColor.isValid()){
        currColorClrz = tmpColor;
    }
    tmpColorizeBinMask = colorizeBinMask.clone();
    tmpColorizeForOut = colorizeForOut.clone();

    qApp->setOverrideCursor( Qt::CrossCursor );
    imshow("Colorize", colorizeBinMask);
    setMouseCallback("Colorize", mouseWrapper, this);
    HWND hwnd = (HWND)cvGetWindowHandle("Colorize");
    while(IsWindowVisible(hwnd))
    {
        char key = (char)waitKey(0);
        if (key == 's')
        {
            colorizeBinMask = tmpColorizeBinMask.clone();
            colorizeForOut = tmpColorizeForOut.clone();
            keyHandled = true;
            break;
        }
        if (key == 'q')
        {
            qApp->restoreOverrideCursor();
            tmpColor = QColorDialog::getColor(currColorClrz, this, "Выбор цвета контура");
            if (tmpColor.isValid()){
                currColorClrz = tmpColor;
            }
            qApp->setOverrideCursor( Qt::CrossCursor );
        }
        if (key == 27)
        {
            keyHandled = true;
            break;
        }
        if (!(keyHandled)){
            colorizeBinMask = tmpColorizeBinMask.clone();
            colorizeForOut = tmpColorizeForOut.clone();
        }
    }

    qApp->restoreOverrideCursor();
    destroyAllWindows();
    ui->warningLabel->hide();
    ui->backSelect->setDisabled(false);
    ui->changeColorBtn->setDisabled(false);
    ui->colorizeBtn->setDisabled(false);
    ui->loadImageBut->setDisabled(false);
    ui->saveBtn->setDisabled(false);
    changeCurrentImage();
}

void mouseWrapper( int event, int x, int y, int flags, void* param )
{
    PPWindow * mainWin = (PPWindow *)(param);
    mainWin->onMouseColorize(event,x,y,flags,0);
}

void PPWindow::onMouseColorize(int event, int x, int y, int flag, void*){
    if ((event == EVENT_MOUSEMOVE) && (flag == EVENT_FLAG_LBUTTON))
    {
        QRgb tmpColor = currColorClrz.rgb();
        Scalar cvTmpColor(qBlue(tmpColor), qGreen(tmpColor), qRed(tmpColor));
        circle(tmpColorizeBinMask, Point(x, y), 10, cvTmpColor, -1);
        circle(tmpColorizeForOut, Point(x, y), 10, cvTmpColor, -1);
        imshow("Colorize", tmpColorizeBinMask);
    } else
        if (event == EVENT_LBUTTONUP)
        {
            Vec3b tmpColorMat;
            tmpColorMat[0] = 0;
            tmpColorMat[1] = 0;
            tmpColorMat[2] = 0;

            for (int y = 0; y < tmpColorizeBinMask.rows; ++y) {
                for (int x = 0; x < tmpColorizeBinMask.cols; ++x) {
                    if (objBinMask.at<uchar>(y, x) == 0){
                        tmpColorizeBinMask.at<Vec3b>(y, x) = tmpColorMat;
                    }
                }
            }
            imshow("Colorize", tmpColorizeBinMask);
        }
}

void PPWindow::saveImage(){
    QString imagesavename = QFileDialog::getSaveFileName(0, "Сохранение автарки",
                                                         QDir::current().absolutePath(),
                                                         "*.jpg")
            .toLocal8Bit();

    if (!(imagesavename.isEmpty()) ){
        std::string matName = imagesavename.toStdString();
        if (!setObject){
            currShowMat = currBack.clone();
        } else {
            currShowMat = currBack.clone();
            for (int y = 0; y < currShowMat.rows; ++y) {
                for (int x = 0; x < currShowMat.cols; ++x) {
                    if ((colorizeForOut.at<Vec3b>(y, x)[0] != 0) ||
                            (colorizeForOut.at<Vec3b>(y, x)[1] != 0) ||
                            (colorizeForOut.at<Vec3b>(y, x)[2] != 5)){
                        currShowMat.at<Vec3b>(y, x)[0] = colorizeForOut.at<Vec3b>(y, x)[0];
                        currShowMat.at<Vec3b>(y, x)[1] = colorizeForOut.at<Vec3b>(y, x)[1];
                        currShowMat.at<Vec3b>(y, x)[2] = colorizeForOut.at<Vec3b>(y, x)[2];
                    }
                }
            }
            QRgb tmpColor = currColorClust.rgb();
            for (int y = 0; y < currShowMat.rows; ++y) {
                for (int x = 0; x < currShowMat.cols; ++x) {
                    if (objBinMask.at<uchar>(y, x) == 0){
                        currShowMat.at<Vec3b>(y, x)[0] = qBlue(tmpColor);
                        currShowMat.at<Vec3b>(y, x)[1] = qGreen(tmpColor);
                        currShowMat.at<Vec3b>(y, x)[2] = qRed(tmpColor);
                    }
                }
            }
        }
        imwrite(matName, currShowMat);
    }
}
