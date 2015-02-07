#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlabel.h>
#include <iostream>
#include <qpainter.h>
#include <qimage.h>
#include <QMouseEvent>

namespace Ui
{
    class MainWindow;
}

class VoronoiParticle
{
public:
    VoronoiParticle(int X, int Y, const VoronoiParticle& parent) : _x(X), _y(Y), _cx(parent._cx), _cy(parent._cy), _name(parent._name), _id(parent._id), _isChild(true), _isMoved(false) { }
    VoronoiParticle(int X, int Y, std::string Name="") : _x(X), _y(Y), _cx(X), _cy(Y), _name(Name), _id(_aiID), _isChild(false), _isMoved(false) { _aiID++; }
    VoronoiParticle(int X, int Y, int cX, int cY, std::string Name="") : _x(X), _y(Y), _cx(cX), _cy(cY), _name(Name), _id(_aiID), _isChild(false), _isMoved(true) { _aiID++; }

    int x()  const { return _x  + 100 + 450; }
    int y()  const { return 100 + 300 - _y; }
    int cx() const { return _cx + 100 + 450; }
    int cy() const { return 100 + 300 - _cy; }
    unsigned int id() const { return _id; }
    bool isChild() const { return _isChild; }
    bool isMoved() const { return _isChild || _isMoved; }

    int xraw()  const { return _x; }
    int yraw()  const { return _y; }
    int cxraw() const { return _cx; }
    int cyraw() const { return _cy; }

    int _x, _y, _cx, _cy;
    std::string _name;
    unsigned int _id;

private:
    bool _isChild;
    bool _isMoved;
    static unsigned int _aiID;
};

class Monitor : public QLabel
{
    Q_OBJECT
public:

    explicit Monitor(QWidget *parent = 0) : QLabel(parent) {}
    ~Monitor() {}

signals:
    void addParticle(int x, int y, int cx, int cy);
    void mMove(int, int, int, int);

private:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

    int sX, sY;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QPainter* qpn;
    QImage* qmg;
    Monitor* monitor;
    std::vector<VoronoiParticle> particles;
    const int SNAP_MAX;
    QString address;
    bool hasSaved;

    void drawField();
    void refreshUI();
    void drawParticles();

    unsigned int nearestID(int x, int y);
    unsigned int nearBy(int x, int y, float &d);

    void addParticle(const VoronoiParticle& p);
    void saveConfig(const std::string& add);
    void loadConfig(const std::string& add);

private slots:
    void getParticle(int x, int y, int cx, int cy);
    void getMouseMove(int, int, int, int);
    void on_b_id_editingFinished();
    void on_pushButton_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
};

#endif // MAINWINDOW_H
