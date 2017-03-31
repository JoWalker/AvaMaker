#ifndef PPWINDOW_H
#define PPWINDOW_H

#include <QWidget>
#include <opencv2\opencv.hpp>
#include <QColor>
#include <QMovie>

namespace Ui {
class PPWindow;
}

class PPWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PPWindow(QWidget *parent = 0);
    void onMouseColorize(int event, int x, int y, int flag, void*);
    ~PPWindow();

private:
    Ui::PPWindow *ui;    
    void setCurrentBackground(QString name);
    void changeCurrentImage();
    void invertSelection();
    void loadImageWithObj();
    cv::Mat loadFromQrc(QString qrc, int flag = cv::IMREAD_COLOR);
    void changeColorClust();
    void colorizeSpaces();
    void saveImage();

    cv::Mat currBack;
    cv::Mat currShowMat;
    cv::Mat startObjImg;
    cv::Mat objBinMask;
    cv::Mat colorizeBinMask;
    cv::Mat colorizeForOut;
    cv::Mat tmpColorizeBinMask;
    cv::Mat tmpColorizeForOut;
    bool setObject;
    QImage currShowQImg;
    QPixmap currShowQPixmap;
    QColor currColorClust;
    QColor currColorClrz;
};

void mouseWrapper( int event, int x, int y, int flags, void* param );

#endif // PPWINDOW_H
