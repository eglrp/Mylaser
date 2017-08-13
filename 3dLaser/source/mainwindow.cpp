#include <QMessageBox>
#include <Windows.h>
#include <qdebug>
#include <QFileDialog>

#include "macro.h"
#include "mainwindow.h"

//#include "adt8933.h"


using namespace std;




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName(organization);
    QCoreApplication::setApplicationName(appName);

    ParamWindow::readParamFromRegistry(sharedPara.get());

    initOSG();
    initActions();
    updateRecentFileAndPathActions();
    initStatusBar();
    initTimer();
    initSignalsAndSlots();

    updateDockWidget(); //更新侧边栏

    initCtrlBoard();    //初始化控制卡
}

MainWindow::~MainWindow()
{
    // 结束计时
    endTimer();
    // 参数写入注册表
    ParamWindow::writeParamToRegistry(sharedPara.get());

    delete ui;
}

// Open points cloud file(.dxf)
bool MainWindow::on_FileOpen(QString fileName)
{
    QStringList fileNameList;
    if(fileName.isEmpty())
        fileNameList = QFileDialog::getOpenFileNames(this,tr("导入点云"), lastUsedDirectory.path(), MI.inputPointCloudFilters.join(";;"));
    else
        fileNameList.push_back(fileName);

    if (fileNameList.isEmpty())	return false;
    else
    {
        //Save path away so we can use it again
        QString path = fileNameList.first();
        path.truncate(path.lastIndexOf("/"));
        lastUsedDirectory.setPath(path);

        if(!lastUsedDirectory.path().isEmpty())
            saveRecentPathList(lastUsedDirectory.path());
    }

    QTime time;
    time.start();
    foreach(fileName,fileNameList)
    {
        QFileInfo fi(fileName);
        QString extension = fi.suffix();
        bool b;
        b = MI.allKnowPointCloudInputFormats.contains(extension.toLower(),Qt::CaseSensitive);
        if(!b)
        {
            QString errorMsgFormat(tr("无法打开文件:\n\"%1\"\n\n详细信息: 文件格式 ") + extension + tr(" 不支持."));
            QMessageBox::critical(this,tr("文件打开错误"),errorMsgFormat.arg(fileName));
            return false;
        }

        osg::ref_ptr<osg::Group> group = MI.openMesh(extension,fileName);
        //以上各节点name均已不为空
        if(group)
        {
            osg::ref_ptr<osg::Vec3Array> points=getVertexArray(group.get());
            if(points->size()>0)//添加点云并切换至点云显示模式
                addPointsToPointCloudGroup(pointCloudGroup.get(), points.get(),false);
        }
        else
        {
            QMessageBox::critical(this,tr("错误"), fileName + tr("  内容不被支持！"));
            return false;
        }

        saveRecentFileList(fileName);
    }

    updateStatusBar(QString::number(time.elapsed()/1000.f)+"s");
    return true;
}

void MainWindow::on_ClearAll()
{
    pointCloudGroup->removeChildren(0, pointCloudGroup->getNumChildren());
}

void MainWindow::on_OpenRecentFile()
{
    QAction *action = (QAction *)(sender());
    QString fileName = action->data().toString();
    QFileInfo fi(fileName);
    if(fi.isFile())
        on_FileOpen(fileName);
    else
        saveRecentFileList(fileName, true);
}

void MainWindow::on_OpenRecentPath()
{
    QAction *action = (QAction *)(sender());
    QString path = action->data().toString();
    QDir dir(path);
    QFileInfo fi(path);
    if(dir.exists() && fi.isDir())
    {
        lastUsedDirectory.setPath(path);
        on_FileOpen();
    }
    else
        saveRecentPathList(path, true);
}

void MainWindow::on_quitApplication()
{
    this->close();
}

void MainWindow::on_ParameterSetting()
{
    ParamWindow paraDialog(sharedPara.get());
    if(QDialog::Accepted == paraDialog.exec())
    {
        updateDockWidget(); //更新侧边栏
        updateRefShape(createCrystalFrame(sharedPara->crystalSize, 0.f));
    }
}

//相机位置方向
void MainWindow::on_setViewDirection(ViewDirection direction)
{
    QAction *action = (QAction*) sender();
    if(action == ui->action_FrontView)
        direction = FRONTVIEW;
//    else if(action == ui->action_LeftView)
//        direction = LEFTVIEW;
    else if(action == ui->action_RightView)
        direction = RIGHTVIEW;
    else if(action == ui->action_TopView)
        direction = TOPVIEW;

    curViewer->setViewDirection(direction);
}

void MainWindow::on_StartEngraving()
{

}

void MainWindow::on_PauseEngraving()
{

}

void MainWindow::on_StopEngraving()
{

}

void MainWindow::on_LaserOnOff(bool checked)
{
    const QString sLaserOn = tr("开激光");
    const QString sLaserOff = tr("关激光");
    if(checked)
    {
        ui->pb_LaserOnOff->setText(sLaserOff);

        ctrlCard.Set_Pwm(sharedPara->testFrequency, sharedPara->testRatio);

//        for(int i=0; i<1000; i++)
//        {
//            fifo_set_laser_control(0,1);
//            int nNum;
//            read_fifo(0,&nNum);
//            qDebug("nNum: %d", nNum);
//            while(nNum >= 2048 - 1)
//            {
//                read_fifo(0,&nNum);
//            }
//            fifo_set_pwm_freq1(0,(int)(1.0/sharedPara->testFrequency*1000000*8));
//            read_fifo(0,&nNum);
//            qDebug("nNum: %d", nNum);
//            fifo_set_pwm_freq2(0,(int)(1.0/sharedPara->testFrequency*1000000*sharedPara->testRatio*8));
//            read_fifo(0,&nNum);
//            qDebug("nNum: %d", nNum);
//        }
    }
    else
    {
        ui->pb_LaserOnOff->setText(sLaserOn);

        ctrlCard.Close_Laser();  //关激光
    }
}

void MainWindow::on_MoveToStartPos()
{
    if(!bHomed)
    {
        QMessageBox::warning(this, tr("警告"), tr("机器尚未复位！"));
        return;
    }

    float xPos=sharedPara->initOffset[AXISX];
    float yPos=sharedPara->initOffset[AXISY];
    float zPos=/*sharedPara->initOffset[AXISZ]*/50.f;

    xPos -= sharedPara->crystalSize.x()*0.5f;
    yPos += sharedPara->crystalSize.y()*0.5f;

    curPosition = getCurPos();
    xPos -= curPosition.x();
    yPos -= curPosition.y();
    zPos -= curPosition.z();

    axisMoveTo(AXISX, xPos);
    axisMoveTo(AXISY, yPos);
    axisMoveTo(AXISZ, zPos);
    bMotorRunning = true;
    Sleep(20);
}

void MainWindow::on_MoveToHomePos()
{
    float xPos=sharedPara->initOffset[AXISX];
    float yPos=sharedPara->initOffset[AXISY];
    float zPos=/*sharedPara->initOffset[AXISZ]*/50.f;

    axisMoveToDirection(XN);   //左
    axisMoveToDirection(YN);   //前
    axisMoveToDirection(ZN);   //下
    bMotorRunning = true;
    Sleep(20);
    ui->gb_EngravingCtrl->setEnabled(false);
    ui->gb_PlatformCtrl->setEnabled(false);
    while(true) //同步运动
    {
        QCoreApplication::processEvents();

        if(bMotorRunning) Sleep(10);
        else break;
    }

    ctrlCard.Clear_Count(); //当前位置置0

    axisMoveTo(AXISX, xPos);
    axisMoveTo(AXISY, yPos);
    axisMoveTo(AXISZ, zPos);
    bMotorRunning = true;
    Sleep(20);
    while(true) //同步运动
    {
        QCoreApplication::processEvents();

        if(bMotorRunning) Sleep(10);
        else break;
    }

    ui->gb_EngravingCtrl->setEnabled(true);
    ui->gb_PlatformCtrl->setEnabled(true);
    bHomed = true;
}

void MainWindow::on_StopMoving()
{
    stopMove(DEC_STOP);
}

void MainWindow::on_MoveXP()
{
    axisMoveToDirection(XP);
}

void MainWindow::on_MoveXN()
{
    axisMoveToDirection(XN);
}

void MainWindow::on_MoveYP()
{
    axisMoveToDirection(YP);
}

void MainWindow::on_MoveYN()
{
    axisMoveToDirection(YN);
}

void MainWindow::on_MoveZP()
{
    axisMoveToDirection(ZP);
}

void MainWindow::on_MoveZN()
{
    axisMoveToDirection(ZN);
}

void MainWindow::on_OperationModeChanged()
{

}

void MainWindow::on_Preview()
{

}

void MainWindow::on_Apply()
{

}

void MainWindow::on_RunningTimer()
{
    tdRunningTime.incTick();
    if(isLaserOn) tdEngravingTime.incTick();

    QString str = QString("总运行时间: %1:%2:%3")
            .arg(QString::number(tdRunningTime.hour))
            .arg(tdRunningTime.minute,2,10,QChar('0'))
            .arg(tdRunningTime.second,2,10,QChar('0'));
    lb_totalRunTime->setText(str);

    str = QString("总雕刻时间: %1:%2:%3")
            .arg(QString::number(tdEngravingTime.hour))
            .arg(tdEngravingTime.minute,2,10,QChar('0'))
            .arg(tdEngravingTime.second,2,10,QChar('0'));
    lb_totalEngravingTime->setText(str);
}

void MainWindow::on_MotionMonitoringTimer()
{
    bMotorRunning = motorIsRunning();
    ui->pb_MoveToStartPos->setEnabled(!bMotorRunning);
    ui->pb_MoveToHome->setEnabled(!bMotorRunning);

    // 更新界面当前位置
    curPosition = getCurPos();
    QLabel *lb[MAXNUM_MOTOR] = {ui->lb_PlatCurPosValX, ui->lb_PlatCurPosValY, ui->lb_PlatCurPosValZ};
    for(int i=1; i<=MAXNUM_MOTOR; i++)
    {
        lb[i-1]->setText(QString::number(curPosition._v[i-1]));
    }
}

void MainWindow::on_setAxesVisible(bool visible)
{
    curViewer->setAxesVisible(visible);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //resize时更新投影（否则 由于初始size太小而无法正确初始化正投影）
    osg::ref_ptr<MyTrackballManipulator> manipulator =
            dynamic_cast<MyTrackballManipulator *>(curViewer->getCameraManipulator());
    if(manipulator->isOrthoProjection())
        initProjectionAsOrtho();
}

void MainWindow::initStatusBar()
{
    hSpacer1 = new QSpacerItem(90,25,QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->statusBar->layout()->addItem(hSpacer1);

    lb_totalRunTime = new QLabel(this);
    lb_totalRunTime->setAlignment(Qt::AlignLeft);
    lb_totalRunTime->setMaximumWidth(200);
    lb_totalRunTime->setMinimumWidth(200);

    lb_totalEngravingTime = new QLabel(this);
    lb_totalEngravingTime->setAlignment(Qt::AlignLeft);
    lb_totalEngravingTime->setMaximumWidth(200);
    lb_totalEngravingTime->setMinimumWidth(200);

    ui->statusBar->layout()->addWidget(lb_totalRunTime);
    ui->statusBar->layout()->addWidget(lb_totalEngravingTime);
}
void MainWindow::initTimer()
{
    connect(&runningTimer, SIGNAL(timeout()), this, SLOT(on_RunningTimer()));
    runningTimer.setInterval(TIMESPAN);
    runningTimer.start();

    connect(&motionMonitoringTimer, SIGNAL(timeout()), this, SLOT(on_MotionMonitoringTimer()));
    motionMonitoringTimer.setInterval(MONITORINGTIMESPAN);
    motionMonitoringTimer.start();


    // 初始化时间计数器
    QSettings settings;
    settings.beginGroup(g_SoftwareInfo);
    tdRunningTime.hour = settings.value(sRunningTimeH, QVariant((unsigned int)0)).toUInt();
    tdRunningTime.minute = settings.value(sRunningTimeM, QVariant((unsigned int)0)).toUInt();
    tdRunningTime.second = settings.value(sRunningTimeS, QVariant((unsigned int)0)).toUInt();
    tdEngravingTime.hour = settings.value(sEngravingTimeH, QVariant((unsigned int)0)).toUInt();
    tdEngravingTime.minute = settings.value(sEngravingTimeM, QVariant((unsigned int)0)).toUInt();
    tdEngravingTime.second = settings.value(sEngravingTimeS, QVariant((unsigned int)0)).toUInt();
    settings.endGroup();
}

void MainWindow::endTimer()
{
    runningTimer.stop();
    motionMonitoringTimer.stop();

    // 运行时间保存至注册表
    QSettings settings;
    settings.beginGroup(g_SoftwareInfo);
    settings.setValue(sRunningTimeH, QVariant((float)(tdRunningTime.hour)));    //float精度可用957年
    settings.setValue(sRunningTimeM, QVariant((float)(tdRunningTime.minute)));
    settings.setValue(sRunningTimeS, QVariant((float)(tdRunningTime.second)));
    settings.setValue(sEngravingTimeH, QVariant((float)(tdEngravingTime.hour)));
    settings.setValue(sEngravingTimeM, QVariant((float)(tdEngravingTime.minute)));
    settings.setValue(sEngravingTimeS, QVariant((float)(tdEngravingTime.second)));
    settings.endGroup();
    settings.sync();
}

void MainWindow::initSignalsAndSlots()
{
    connect(ui->action_FileOpen, SIGNAL(triggered()), this, SLOT(on_FileOpen()));
    connect(ui->action_ClearAll, SIGNAL(triggered()), this, SLOT(on_ClearAll()));
    connect(ui->action_QuitApp, SIGNAL(triggered()), this, SLOT(on_quitApplication()));

    connect(ui->action_ParameterSettings, SIGNAL(triggered()), this, SLOT(on_ParameterSetting()));

    connect(ui->action_FrontView, SIGNAL(triggered()), this, SLOT(on_setViewDirection()));
    connect(ui->action_RightView, SIGNAL(triggered()), this, SLOT(on_setViewDirection()));
    connect(ui->action_TopView, SIGNAL(triggered()), this, SLOT(on_setViewDirection()));

    /**   侧边栏    **/
    connect(ui->pb_Start, SIGNAL(clicked()), this, SLOT(on_StartEngraving()));
    connect(ui->pb_Pause, SIGNAL(clicked()), this, SLOT(on_PauseEngraving()));
    connect(ui->pb_Stop, SIGNAL(clicked()), this, SLOT(on_StopEngraving()));
    connect(ui->pb_LaserOnOff, SIGNAL(clicked(bool)), this, SLOT(on_LaserOnOff(bool)));

    connect(ui->pb_MoveToStartPos, SIGNAL(clicked(bool)), this, SLOT(on_MoveToStartPos()));
    connect(ui->pb_MoveToHome, SIGNAL(clicked(bool)), this, SLOT(on_MoveToHomePos()));
    connect(ui->pb_StopMoving, SIGNAL(clicked(bool)), this, SLOT(on_StopMoving()));
    connect(ui->pb_MoveXN, SIGNAL(pressed()), this, SLOT(on_MoveXN()));
    connect(ui->pb_MoveXN, SIGNAL(released()), this, SLOT(on_StopMoving()));
    connect(ui->pb_MoveXP, SIGNAL(pressed()), this, SLOT(on_MoveXP()));
    connect(ui->pb_MoveXP, SIGNAL(released()), this, SLOT(on_StopMoving()));
    connect(ui->pb_MoveYN, SIGNAL(pressed()), this, SLOT(on_MoveYN()));
    connect(ui->pb_MoveYN, SIGNAL(released()), this, SLOT(on_StopMoving()));
    connect(ui->pb_MoveYP, SIGNAL(pressed()), this, SLOT(on_MoveYP()));
    connect(ui->pb_MoveYP, SIGNAL(released()), this, SLOT(on_StopMoving()));
    connect(ui->pb_MoveZN, SIGNAL(pressed()), this, SLOT(on_MoveZN()));
    connect(ui->pb_MoveZN, SIGNAL(released()), this, SLOT(on_StopMoving()));
    connect(ui->pb_MoveZP, SIGNAL(pressed()), this, SLOT(on_MoveZP()));
    connect(ui->pb_MoveZP, SIGNAL(released()), this, SLOT(on_StopMoving()));
    connect(ui->rb_ContinousMove, SIGNAL(toggled(bool)), this, SLOT(on_OperationModeChanged()));
    connect(ui->rb_IncrementalMove, SIGNAL(toggled(bool)), this, SLOT(on_OperationModeChanged()));
    connect(ui->pb_Preview, SIGNAL(clicked(bool)), this, SLOT(on_Preview()));
    connect(ui->pb_Apply, SIGNAL(clicked(bool)), this, SLOT(on_Apply()));
}

void MainWindow::initActions()
{
    for(int i=0;i<MAX_NUMFILES;++i)
    {
        recentFileActs[i]=new QAction(this);
        recentFileActs[i]->setVisible(true);
        recentFileActs[i]->setEnabled(true);

        recentPathActs[i]=new QAction(this);
        recentPathActs[i]->setVisible(true);
        recentPathActs[i]->setEnabled(true);
        connect(recentFileActs[i],SIGNAL(triggered()),this,SLOT(on_OpenRecentFile()));
        connect(recentPathActs[i],SIGNAL(triggered()),this,SLOT(on_OpenRecentPath()));

        ui->menu_RecentFileList->addAction(recentFileActs[i]);
        ui->menu_RecentPathList->addAction(recentPathActs[i]);
    }
}

void MainWindow::initOSG()
{
    curViewer = new osgContainer();
    refShape=new osg::Geode;
    pointCloudGroup=new osg::Group;
    this->setCentralWidget(curViewer.get());

    curRoot = curViewer->getRoot();
    curRoot->addChild(refShape.get());
    curRoot->addChild(pointCloudGroup.get());

    initProjectionAsOrtho();
    on_setAxesVisible(true);
    updateRefShape(createCrystalFrame(sharedPara->crystalSize, 0.f));
    updateLighting(true);
}

void MainWindow::initProjectionAsOrtho()
{
    osg::ref_ptr<osg::Camera> camera = curViewer->getCamera();
    osg::ref_ptr<osg::Viewport> viewport=camera->getViewport();
    double left = -viewport->width()*0.1;   //正投影参数
    double right = -left;
    double bottom = -viewport->height()*0.1;
    double top = -bottom;
    double znear = -5000.;
    double zfar = 5000.;

    camera->setProjectionMatrixAsOrtho( left, right, bottom, top, znear, zfar);
}

void MainWindow::updateRefShape(osg::Geode *geode)
{
    curRoot->removeChild(refShape);
    refShape = geode;
    curRoot->addChild(refShape);
}

/// 更新OSG环境光照
void MainWindow::updateLighting(bool brightening)
{
    if(!curViewer) return;
    curViewer->updateLighting(brightening);
}

// 控制卡初始化
int MainWindow::initCtrlBoard()
{
    int rtn = 0;
    QString sFailToInit = tr("控制卡初始化失败!\n详细信息：");
    //*************初始化8940A1卡**************
    rtn = ctrlCard.Init_Board();
    if(rtn == 0)
    {
        QMessageBox::warning(this, tr("错误"), sFailToInit+tr("没有安装ADT8937卡!"));
    }
    else if(rtn == -1)
    {
        QMessageBox::warning(this, tr("错误"), sFailToInit+tr("没有安装端口驱动程序!"));
    }
    else if(rtn == -2)
    {
        QMessageBox::warning(this, tr("错误"), sFailToInit+tr("PCI桥故障!"));
    }

    return rtn;
}

// 轴移动
void MainWindow::axisMoveToDirection(MoveDirection dir)
{
    float pos=0;
    const float cPos = 1000.f;  //mm,给定的某方向的较大值
    if(dir == XP)
    {
        pos = cPos;
        axisMoveTo(AXISX, pos);
    }
    else if(dir == XN)
    {
        pos = -cPos;
        axisMoveTo(AXISX, pos);
    }
    else if(dir == YP)
    {
        pos = cPos;
        axisMoveTo(AXISY, pos);
    }
    else if(dir == YN)
    {
        pos = -cPos;
        axisMoveTo(AXISY, pos);
    }
    else if(dir == ZP)
    {
        pos = cPos;
        axisMoveTo(AXISZ, pos);
    }
    else if(dir == ZN)
    {
        pos = -cPos;
        axisMoveTo(AXISZ, pos);
    }
}

// 停止运动
/// @param mode: SUDDEN_STOP和DEC_STOP两种
void MainWindow::stopMove(int mode)
{
    for(int i=1; i<=MAXNUM_MOTOR; i++)
        ctrlCard.StopRun(i, mode);
}

// 运动至
/// @param pos: 移动到的位置，单位：mm
void MainWindow::axisMoveTo(AXIS axis, float pos)
{
    long lVStart, lSpeed, lAcc, lPos;
    AXIS mappedAxis;    //轴映射; 雕刻平台的方向X/Y/Z(0/1/2)映射到控制卡的控制轴Z/Y/X，pos正负控制轴的方向

    if(axis == AXISX)
    {
        lVStart = sharedPara->startSpeed[AXISX];
        lSpeed = sharedPara->runSpeed[AXISX];
        lAcc = sharedPara->acc[AXISX];
        lPos = (long)(-pos*sharedPara->motorRatio[AXISX]);
        mappedAxis = AXISZ;
    }
    else if(axis == AXISY)
    {
        lVStart = sharedPara->startSpeed[AXISY];
        lSpeed = sharedPara->runSpeed[AXISY];
        lAcc = sharedPara->acc[AXISY];
        lPos = (long)(-pos*sharedPara->motorRatio[AXISY]);
        mappedAxis = AXISY;
    }
    else if(axis == AXISZ)
    {
        lVStart = sharedPara->startSpeed[AXISZ];
        lSpeed = sharedPara->runSpeed[AXISZ];
        lAcc = sharedPara->acc[AXISZ];
        lPos = (long)(pos*sharedPara->motorRatio[AXISZ]);
        mappedAxis = AXISX;
    }

    ctrlCard.Setup_Speed(mappedAxis+1, lVStart, lSpeed, lAcc);
    ctrlCard.Axis_Pmove(mappedAxis+1, lPos);
}

osg::Vec3d MainWindow::getCurPos()
{
    osg::Vec3d pos;
    AXIS oAxis[MAXNUM_MOTOR] = {AXISZ, AXISY, AXISX};   //电机轴往回映射至笛卡尔坐标系轴
    long logicPos, actualPos, speed;
    int k=1;
    for(int i=1; i<=MAXNUM_MOTOR; i++)
    {
        ctrlCard.Get_CurrentInf(i, logicPos, actualPos, speed);
        if(i!=1) k=-1;
        pos._v[oAxis[i-1]] = (double)k*logicPos/sharedPara->motorRatio[i-1];
    }
    return pos;
}

// 是否有电机正在运行
// 返回 true：有电机正在运行； false:所有电机均未在运行
bool MainWindow::motorIsRunning()
{
    int value[MAXNUM_MOTOR]={0,0,0};
    bool stopped = true;
    bool running = false;
    for(int i=1; i<=MAXNUM_MOTOR; i++)
    {
        ctrlCard.Get_Status(i, value[i-1], 0);

        running = running || value[i-1]!=0;
        stopped = stopped && value[i-1]==0;
    }

    if(!running && stopped) return false;
    return true;
}


/// 保存最近打开的文件列表
/// @param fileName:将要添加/移除的文件
/// @param isRemove:是否是移除当前fileName项，是：则移除； 否：则为添加该项
void MainWindow::saveRecentFileList(const QString &fileName, bool isRemove)
{
    QSettings settings;
    settings.beginGroup(g_SoftwareInfo);
    QStringList files = settings.value(sRecentFileList).toStringList();
    files.removeAll(fileName);
    if(!isRemove)   //不是移除该项，则即为添加该项
        files.prepend(fileName);
    while(files.size()>MAX_NUMFILES)
        files.removeLast();

    settings.setValue(sRecentFileList, QVariant(files));
    settings.endGroup();

    updateRecentFileAndPathActions();
}

/// 保存最近打开的文件夹
/// @param path:将要添加/移除的文件夹
/// @param isRemove:是否是移除当前path项，是：则移除； 否：则为添加该项
void MainWindow::saveRecentPathList(const QString &path, bool isRemove)
{
    QSettings settings;
    settings.beginGroup(g_SoftwareInfo);
    QStringList paths = settings.value(sRecentPathList).toStringList();
    paths.removeAll(path);
    if(!isRemove)   //不是移除该项，则即为添加该项
        paths.prepend(path);
    while(paths.size()>MAX_NUMFILES)
        paths.removeLast();

    settings.setValue(sRecentPathList, QVariant(paths));
    settings.endGroup();

    updateRecentFileAndPathActions();
}

// 更新最近打开文件及路径列表
void MainWindow::updateRecentFileAndPathActions()
{
    QSettings settings;
    settings.beginGroup(g_SoftwareInfo);
    QStringList files = settings.value(sRecentFileList).toStringList();
    QStringList paths = settings.value(sRecentPathList).toStringList();
    settings.endGroup();

    int numRecentFiles = qMin(files.size(), (int)MAX_NUMFILES);

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAX_NUMFILES; ++j)
        recentFileActs[j]->setVisible(false);


    int numRecentPaths = qMin(paths.size(), (int)MAX_NUMFILES);

    for(int i=0; i<numRecentPaths; ++i)
    {
        QString text = QString("&%1 %2").arg(i + 1).arg(paths[i]);
        recentPathActs[i]->setText(text);
        recentPathActs[i]->setData(paths[i]);
        recentPathActs[i]->setVisible(true);
    }
    for (int j = numRecentPaths; j < MAX_NUMFILES; ++j)
        recentPathActs[j]->setVisible(false);
}

osg::Geode *MainWindow::createCrystalFrame(osg::Vec3 size, float zRot)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors=new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.f,1.f,0.f,1.f));
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geode->addDrawable(geom);

    //    if(type==BasicSettingsDialog::BOX)
    {
        v->push_back(osg::Vec3(-0.5f*size.x(), -0.5f*size.y(), -0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), -0.5f*size.y(), -0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), 0.5f*size.y(), -0.5f*size.z()));
        v->push_back(osg::Vec3(-0.5f*size.x(), 0.5f*size.y(), -0.5f*size.z()));

        v->push_back(osg::Vec3(-0.5f*size.x(), -0.5f*size.y(), 0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), -0.5f*size.y(), 0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), 0.5f*size.y(), 0.5f*size.z()));
        v->push_back(osg::Vec3(-0.5f*size.x(), 0.5f*size.y(), 0.5f*size.z()));

        if(zRot != 0)
        {
            osg::Matrix m = osg::Matrix::rotate(osg::DegreesToRadians(zRot), osg::Vec3(0.f,0.f,1.f));
            for(unsigned int i=0; i<v->size(); ++i)
            {
                osg::Vec4 vt4(v->at(i),1.0f);
                vt4=vt4*m;
                v->at(i) = osg::Vec3(vt4.x(),vt4.y(),vt4.z());
            }
        }

        geom->setVertexArray(v.get());
        osg::ref_ptr<osg::DrawElementsUInt> lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
        for(int i=0; i<8; ++i)
        {
            int j= i+1;
            if(i==3) j=0;
            else if(i==7) j=4;
            lines->push_back(i);
            lines->push_back(j);
        }
        for(int i=0; i<4; ++i)
        {
            lines->push_back(i);
            lines->push_back(i+4);
        }
        geom->addPrimitiveSet(lines);
    }
//    else if(type==BasicSettingsDialog::CYLINDER)
//    {
//        float defaultAngleStep = 3.0f;//缺省步距角
//        float r = 0.5f*diameter;
//        float x,y,z;
//        float rad;
//        for(int i=0; i<2; ++i)
//        {
//            if(i==0) z=-0.5f*height;
//            else z=0.5f*height;
//            for(float agl=0.f; agl<=360.f; agl+=defaultAngleStep)
//            {
//                rad = osg::DegreesToRadians(agl);
//                x = r*std::cos(rad);
//                y = r*std::sin(rad);
//                v->push_back(osg::Vec3(x,y,z));
//            }
//        }
//        geom->setVertexArray(v.get());
//        geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, v->size()/2)); // down
//        geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, v->size()/2, v->size()/2)); // up
//    }
    return geode.release();
}

osg::BoundingBox MainWindow::getBoundingBox(osg::Node *node)
{
    osg::ComputeBoundsVisitor cbv;
    node->accept(cbv);
    return cbv.getBoundingBox();
}

osg::Vec3Array *MainWindow::getVertexArray(osg::Node *node)
{
    vertexExtractor ve;
    node->accept(ve);
    return ve.getAllVertexArrayInWorldCoord();
}

unsigned int MainWindow::getNumVertex(osg::Node *node)
{
    osg::ref_ptr<osg::Vec3Array> points = getVertexArray(node);
    return points->size();
}

void MainWindow::addPointsToPointCloudGroup(osg::Group *pcGroup, osg::Vec3Array *points, bool removeOld)
{
    if(!points || points->size()<=0) return;
    osg::ref_ptr<osg::Geode> geode=new osg::Geode;
    osg::ref_ptr<osg::Geometry> geom=new osg::Geometry;
    geom->setVertexArray(points);
    osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array();
    osg::Vec4 white = osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ca->resize(points->size(), white);
    geom->setColorArray(ca.get());
    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, points->size()));
    geode->addDrawable(geom);
    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    mt->addChild(geode.get());

    if(removeOld)
        pcGroup->removeChildren(0, pcGroup->getNumChildren());
    pcGroup->addChild(mt.get());
}

void MainWindow::updateStatusBar(QString statusMsg)
{
    if(!statusMsg.isEmpty())
        ui->statusBar->showMessage(statusMsg, 2000);
}

void MainWindow::updateDockWidget()
{
    unsigned int numVertices = getNumVertex(pointCloudGroup.get());
    ui->lb_PointNumVal->setText(QString::number(numVertices));

    if(pointCloudGroup && pointCloudGroup->getNumChildren()>0)
    {
        osg::BoundingBox bb = getBoundingBox(pointCloudGroup.get());
        ui->lb_PointMinValX->setText(QString::number(bb._min.x()));
        ui->lb_PointMinValY->setText(QString::number(bb._min.y()));
        ui->lb_PointMinValZ->setText(QString::number(bb._min.z()));
        ui->lb_PointMaxValX->setText(QString::number(bb._max.x()));
        ui->lb_PointMaxValY->setText(QString::number(bb._max.y()));
        ui->lb_PointMaxValZ->setText(QString::number(bb._max.z()));
        ui->lb_PointLenValX->setText(QString::number(bb._max.x()-bb._min.x()));
        ui->lb_PointLenValY->setText(QString::number(bb._max.y()-bb._min.y()));
        ui->lb_PointLenValZ->setText(QString::number(bb._max.z()-bb._min.z()));
    }

    ui->lb_CrystalSizeValX->setText(QString::number((sharedPara->crystalSize.x())));
    ui->lb_CrystalSizeValY->setText(QString::number((sharedPara->crystalSize.y())));
    ui->lb_CrystalSizeValZ->setText(QString::number((sharedPara->crystalSize.z())));

    ui->lb_PlatCurPosValX->setText(QString::number(curPosition.x()));
    ui->lb_PlatCurPosValY->setText(QString::number(curPosition.y()));
    ui->lb_PlatCurPosValZ->setText(QString::number(curPosition.z()));
    ui->lb_RangeValX->setText(QString::number(engravingRange.x()));
    ui->lb_RangeValY->setText(QString::number(engravingRange.y()));
    ui->lb_RangeValZ->setText(QString::number(engravingRange.z()));

    ui->dsb_CrystalSizeX->setValue(sharedPara->crystalSize.x());
    ui->dsb_CrystalSizeY->setValue(sharedPara->crystalSize.y());
    ui->dsb_CrystalSizeZ->setValue(sharedPara->crystalSize.z());
}


