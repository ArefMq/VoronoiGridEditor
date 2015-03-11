#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <qmessagebox.h>
#include <fstream>
#include "filedialog.h"

unsigned int VoronoiParticle::_aiID = 0;

#ifdef FAST_PERFORMANCE
#  define FOR(iterator, initial, cond) \
    for (int iterator=initial; cond; iterator+=2)
#else
#  define FOR(iterator, initial, cond) \
    for (int iterator=0; cond; iterator++)
#endif

void Monitor::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton) return;

    sX = ev->x();
    sY = ev->y();
}

void Monitor::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton) return;
    emit addParticle(sX, sY, ev->x(), ev->y());
}

void Monitor::mouseMoveEvent(QMouseEvent *ev)
{
    emit mMove(sX, sY, ev->x(), ev->y());
}

void MainWindow::getMouseMove(int x1, int y1, int x2, int y2)
{
    if (abs(x1-x2) < SNAP_MAX && abs(y1-y2) < SNAP_MAX) return;

    double theta = atan2(y2-y1 , x2-x1);

    qpn->setPen(QPen(Qt::black, 2));
    qpn->drawLine(x2 - cos(theta)*10, y2 - sin(theta)*10, x1 + cos(theta)*10, y1 + sin(theta)*10);
    qpn->drawLine(x2 - cos(theta)*10, y2 - sin(theta)*10, x2 - cos(theta+0.2)*30, y2 - sin(theta+0.2)*30);
    qpn->drawLine(x2 - cos(theta)*10, y2 - sin(theta)*10, x2 - cos(theta-0.2)*30, y2 - sin(theta-0.2)*30);
    refreshUI();
    qpn->setPen(QPen(Qt::darkGreen, 2));
    qpn->drawLine(x2 - cos(theta)*10, y2 - sin(theta)*10, x1 + cos(theta)*10, y1 + sin(theta)*10);
    qpn->drawLine(x2 - cos(theta)*10, y2 - sin(theta)*10, x2 - cos(theta+0.2)*30, y2 - sin(theta+0.2)*30);
    qpn->drawLine(x2 - cos(theta)*10, y2 - sin(theta)*10, x2 - cos(theta-0.2)*30, y2 - sin(theta-0.2)*30);
}

void MainWindow::getParticle(int x, int y, int cx, int cy)
{
    hasSaved = false;

    x = x-550;
    y = 400-y;
    cx = cx-550;
    cy = 400-cy;

    if (abs(x-cx) < SNAP_MAX && abs(y-cy) < SNAP_MAX)
        addParticle(VoronoiParticle(x, y));
    else if (!particles.size())
        addParticle(VoronoiParticle(x, y, cx, cy));
    else
    {
        float d;
        unsigned int nid = nearBy(cx, cy, d);
        const VoronoiParticle& p = particles[nid];

        if (d < SNAP_MAX)
            addParticle(VoronoiParticle(x, y, p));
        else
            addParticle(VoronoiParticle(x, y, cx, cy));

    }

    drawField();
    drawParticles();
    refreshUI();
}

void MainWindow::addParticle(const VoronoiParticle& p)
{
    ui->b_id->setValue(particles.size());
    ui->b_x->setValue(p.xraw());
    ui->b_y->setValue(p.yraw());
    ui->b_cx->setValue(p.cxraw());
    ui->b_cy->setValue(p.cyraw());
    ui->b_name->setText("");
    ui->b_par->setValue(p.id());

    particles.push_back(p);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    SNAP_MAX(30),
    address(""),
    hasSaved(true)
{
    ui->setupUi(this);

    monitor = new Monitor(ui->centralWidget);
    monitor->setObjectName(QString::fromUtf8("monitor"));
    monitor->setGeometry(QRect(10, 10, 1100, 800));
    monitor->setFrameShape(QFrame::Box);
    connect(monitor, SIGNAL(addParticle(int,int,int,int)), this, SLOT(getParticle(int,int,int,int)));
    connect(monitor, SIGNAL(mMove(int,int,int,int)), this, SLOT(getMouseMove(int,int,int,int)));

    qmg = new QImage(1100, 800, QImage::Format_RGB888);
    qpn = new QPainter(qmg);

    drawField();
    refreshUI();
}

MainWindow::~MainWindow()
{
    delete qpn;
    delete qmg;
    delete ui;
}

void MainWindow::drawField()
{
    //-- Carpet
    qpn->setBrush(Qt::darkGreen);
    qpn->drawRect(0, 0, 1100, 800);

    //-- Goals
    qpn->setPen(Qt::yellow);
    qpn->setBrush(QBrush(Qt::yellow, Qt::Dense6Pattern));
    qpn->drawRect(50, 325, 50, 150);
    qpn->setBrush(QBrush(Qt::blue, Qt::Dense6Pattern));
    qpn->drawRect(1000, 325, 50, 150);

    //-- Lines
    qpn->setBrush(Qt::NoBrush);
    qpn->setPen(QPen(Qt::white, 5));
    qpn->drawRect(100, 100, 900, 600);
    qpn->drawRect(100, 290, 60, 220); //-- Penalty Box
    qpn->drawRect(940, 290, 60, 220); //-- Penalty Box
    qpn->drawLine(550, 100, 550, 700); //-- Half Line
    qpn->drawEllipse(QPoint(550, 400), 75, 75); //-- Circle
    qpn->drawEllipse(QPoint(230, 400), 5, 5); //-- Cross
    qpn->drawEllipse(QPoint(870, 400), 5, 5); //-- Cross

    //-- Self Goal Posts
    qpn->setBrush(Qt::yellow);
    qpn->setPen(Qt::yellow);
    qpn->drawEllipse(QPoint(100, 325), 5, 5);
    qpn->drawEllipse(QPoint(100, 475), 5, 5);

    //-- Opp. Goal Posts
    qpn->setBrush(Qt::yellow);
    qpn->setPen(Qt::yellow);
    qpn->drawEllipse(QPoint(1000, 325), 5, 5);
    qpn->drawEllipse(QPoint(1000, 475), 5, 5);
}

void MainWindow::drawParticles()
{
    if (!particles.size()) return;

    //-- Draw Centers
    qpn->setBrush(Qt::darkBlue);
    qpn->setPen(QPen(Qt::blue, 3));
    for (std::vector<VoronoiParticle>::const_iterator p=particles.begin(); p<particles.end(); p++)
        if (p->isMoved())
            qpn->drawPoint(p->cx(), p->cy());

    //-- Draw Arrows
    qpn->setBrush(Qt::NoBrush);
    qpn->setPen(QPen(Qt::black, 2));

    for (std::vector<VoronoiParticle>::const_iterator p=particles.begin(); p<particles.end(); p++)
        if (p->isMoved())
        {
            double theta = atan2(p->cy()-p->y() , p->cx()-p->x());

            qpn->drawLine(p->cx() - cos(theta)*10, p->cy() - sin(theta)*10, p->x() + cos(theta)*10, p->y() + sin(theta)*10);
            qpn->drawLine(p->cx() - cos(theta)*10, p->cy() - sin(theta)*10, p->cx() - cos(theta+0.2)*30, p->cy() - sin(theta+0.2)*30);
            qpn->drawLine(p->cx() - cos(theta)*10, p->cy() - sin(theta)*10, p->cx() - cos(theta-0.2)*30, p->cy() - sin(theta-0.2)*30);
        }

    //-- Draw Particles
    qpn->setBrush(Qt::darkRed);
    qpn->setPen(QPen(Qt::red, 5));
    for (std::vector<VoronoiParticle>::const_iterator p=particles.begin(); p<particles.end(); p++)
        qpn->drawPoint(p->x(), p->y());

    //-- Draw texts
    qpn->setBrush(Qt::NoBrush);
    qpn->setPen(QPen(Qt::black));
    for (std::vector<VoronoiParticle>::const_iterator p=particles.begin(); p<particles.end(); p++)
        if (p->_name != "")
            qpn->drawText(p->x()-5, p->y()+15, QString::fromStdString(p->_name));

    //-- Draw Borders
    qpn->setPen(QPen(Qt::green, 1));
    FOR (x, 0, x<qmg->width())
//    for (int x=0; x<qmg->width(); x++)
    {
        unsigned int lastID = 0;
        FOR (y, 0, y<qmg->height())
//        for (int y=0; y<qmg->height(); y++)
        {
            unsigned int id = nearestID(x, y);
            if (lastID == id)
                continue;

            qpn->drawPoint(x, y);
            lastID = id;
        }
    }

    FOR (y, 1, y<qmg->height()-2)
//    for (int y=1; y<qmg->height(); y++)
    {
        unsigned int lastID = 0;
        FOR (x, 1, x<qmg->width()-2)
//        for (int x=1; x<qmg->width(); x++)
        {
            unsigned int id = nearestID(x, y);
            if (lastID == id)
                continue;

            qpn->drawPoint(x, y);
            lastID = id;
        }
    }
}

unsigned int MainWindow::nearestID(int x, int y)
{
    float distance = 9999999999;
    unsigned int id = -1;
    for (std::vector<VoronoiParticle>::const_iterator p=particles.begin(); p<particles.end(); p++)
    {
        const float d = (x-p->x())*(x-p->x()) + (y-p->y())*(y-p->y());

        if (distance > d)
        {
            distance = d;
            id = p->id();
        }
    }

    return id;
}

unsigned int MainWindow::nearBy(int x, int y, float& d)
{
    float distance = 9999999999;
    unsigned int id = -1;
    for (unsigned int i=0; i<particles.size(); i++)
    {
        const VoronoiParticle& p = particles[i];
        const float d1 = sqrt((x-p.xraw())*(x-p.xraw()) + (y-p.yraw())*(y-p.yraw()));
        const float d2 = sqrt((x-p.cxraw())*(x-p.cxraw()) + (y-p.cyraw())*(y-p.cyraw()));

        if (distance > d1)
        {
            distance = d1;
            id = i;
        }

        if (distance > d2)
        {
            distance = d2;
            id = i;
        }
    }

    d = distance;
    return id;
}

void MainWindow::refreshUI()
{
    monitor->setPixmap(QPixmap::fromImage(*qmg));
}

void MainWindow::on_b_id_editingFinished()
{
    if ((unsigned)ui->b_id->value() >= particles.size()) return;

    const VoronoiParticle& p = particles[ui->b_id->value()];
    ui->b_x->setValue(p.xraw());
    ui->b_y->setValue(p.yraw());
    ui->b_cx->setValue(p.cxraw());
    ui->b_cy->setValue(p.cyraw());
    ui->b_name->setText(QString::fromStdString(p._name));
    ui->b_par->setValue(p.id());
}

void MainWindow::on_pushButton_clicked()
{
    if ((unsigned)ui->b_id->value() >= particles.size()) return;
    VoronoiParticle& p = particles[ui->b_id->value()];

    if ((unsigned)ui->b_par->value() < particles.size() && p.id() != (unsigned)ui->b_par->value())
    {
        p = VoronoiParticle(ui->b_x->value(), ui->b_y->value(), particles[ui->b_par->value()]);
        ui->b_cx->setValue(p.cxraw());
        ui->b_cy->setValue(p.cyraw());
    }
    else
    {
        p._x = ui->b_x->value();
        p._y = ui->b_y->value();
        p._cx = ui->b_cx->value();
        p._cy = ui->b_cy->value();
    }
    p._name = ui->b_name->text().toStdString();

    drawField();
    drawParticles();
    refreshUI();
}

void MainWindow::on_pushButton_4_clicked()
{
    if (!particles.size()) return;

    if (QMessageBox::warning(this, "Warn", "Are you sure you want to clear all points?", QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        particles.clear();
        drawField();
        drawParticles();
        refreshUI();

        hasSaved = true;
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    hasSaved = false;

    const unsigned s = particles.size();
    for (unsigned i=0; i<s; i++)
    {
        VoronoiParticle np = particles[i];
        np._x *= -1;
        np._cx *= -1;
        np._name = (np._name=="")?"":(np._name+"_mx");
        np._id += s;

        particles.push_back(np);
    }

    drawField();
    drawParticles();
    refreshUI();
}

void MainWindow::on_pushButton_6_clicked()
{
    hasSaved = false;

    const unsigned s = particles.size();
    for (unsigned i=0; i<s; i++)
    {
        VoronoiParticle np = particles[i];
        np._y *= -1;
        np._cy *= -1;
        np._name = (np._name=="")?"":(np._name+"_my");
        np._id += s;

        particles.push_back(np);
    }

    drawField();
    drawParticles();
    refreshUI();
}

void MainWindow::saveConfig(const std::string& add)
{
    std::ofstream file(add.c_str(), std::ios::out | std::ios::trunc);
    if (!file)
    {
        std::cerr << "could not write file..." << std::endl;
        return;
    }

    file << "# region, x, y [, cx, cy] [:name]" << std::endl;
    for (std::vector<VoronoiParticle>::const_iterator p=particles.begin(); p<particles.end(); p++)
    {
        file << p->id() << ", " << p->xraw()*10.f << ", " << p->yraw()*10.f;
        if (p->isMoved())
            file << ", " << p->cxraw()*10.f << ", " << p->cyraw()*10.f;

        if (p->_name != "")
            file << " :" << p->_name;

        file << std::endl;
    }
}

void MainWindow::loadConfig(const std::string& add)
{
    class CFGReader {
    public:
        int id, x, y, cx, cy;
        std::string s;
        bool textMode;
        bool isOk;
        char varCounter;

        void pushAValue(char i, int value)
        {
            switch (i)
            {
            case 0:
                id = value;
                break;
            case 1:
                x = value;
                break;
            case 2:
                y = value;
                break;
            case 3:
                cx = value;
                break;
            case 4:
                cy = value;
                break;
            }
        }

        void processLine(const char* str)
        {
            if (!str) return;

            isOk = false;
            s = "";
            id = x = y = cx = cy = 0;
            textMode = false;

            varCounter=0;
            int varValue=0;
            bool neg = false;

            unsigned int i;

            for (i=0; str[i] != '\0'; i++)
            {
                const char c = str[i];

                if (c == '#' || c==';') //-- A Comment
                    break;

                if (c == ' ')
                    continue;

                if (c == ':')
                {
                    textMode = true;
                    break;
                }

                if (c == ',') //-- Push An other
                {
                    if (varCounter < 5)
                        pushAValue(varCounter, neg?(-1*varValue):varValue);
                    varValue = 0;
                    varCounter++;
                    neg = false;
                    continue;
                }

                if (c == '-')
                {
                    neg = true;
                    continue;
                }

                varValue = varValue*10 + (c - 48);
            }

            if (varValue != 0)
            {
                pushAValue(varCounter, neg?(-1*varValue):varValue);
            }

            if (textMode)
            {
                for (i++; str[i] != '\0'; i++)
                {
                    s += str[i];
                }
            }

            if (varCounter < 4)
            {
                cx = x;
                cy = y;
            }

            if (varCounter < 2)
                isOk = false;
            else
                isOk = true;
        }
    } cfgReader;

    std::ifstream file(add.c_str(), std::ios::in);
    if (!file) { std::cerr << "File not found..." << std::endl; return; }

    char sz[255];
    while (!file.eof())
    {
        file.getline(sz, 255);
        cfgReader.processLine(sz);
        if (!cfgReader.isOk) continue;

        VoronoiParticle p = VoronoiParticle(0, 0, "");
        p._x = cfgReader.x/10.f;
        p._y = cfgReader.y/10.f;
        p._cx = cfgReader.cx/10.f;
        p._cy = cfgReader.cy/10.f;
        p._id = cfgReader.id;
        p._name = cfgReader.s;

        std::cout << p.id() << ") " << p.xraw() << ", " << p.yraw() << std::endl;

        particles.push_back(p);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    FileDialog* fd = new FileDialog(address, this);
    fd->exec();
    if (!fd->hasAccepted) return;
    address = fd->address();

    std::cout << "Writing file on " << address.toStdString() << std::endl;
    saveConfig(address.toStdString());
    hasSaved = true;
}

void MainWindow::on_pushButton_3_clicked()
{
    if (!hasSaved && QMessageBox::warning(this, "Warn", "You have un-saved points,\nBy reloading, current points will be lost,\nAre you sure you want to reload?",
                                          QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
        return;

    hasSaved = true;
    particles.clear();

    FileDialog* fd = new FileDialog(address, this);
    fd->exec();
    if (!fd->hasAccepted) return;
    address = fd->address();

    std::cout << "Loading file from " << address.toStdString() << std::endl;
    loadConfig(address.toStdString());

    drawField();
    drawParticles();
    refreshUI();
}
