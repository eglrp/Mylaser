#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <QString>
#include <QMainWindow>
#include <QTimer>

#include <osg/MatrixTransform>
#include <osg/Vec3>
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/Selection>
#include <osgGA/GUIEventAdapter>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/TranslateAxisDragger>
#include <osgManipulator/ScaleAxisDragger>
#include <osgManipulator/CommandManager>
#include <osg/ComputeBoundsVisitor>

#include "osgContainer.h"
#include "ui_mainwindow.h"
#include "meshinterface.h"
#include "parameter.h"
#include "visitor.h"
#include "parawindow.h"
#include "markwindow.h"
#include "mcurvwindow.h"
#include "ctrlcard.h"

#if _MSC_VER >= 1600
   #pragma execution_character_set("utf-8")
#endif


#define TIMESPAN 1000
#define MONITORINGTIMESPAN 100
#define MAX_NUMFILES 10


namespace Ui {
class MainWindow;
}

class TimerData
{
public:
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;

public:
    // 加一秒钟
    inline void incTick()
    {
        if(second+1<60) //秒+1
            second++;
        else            //分钟+1
        {
            if(minute<60)//分钟+1
            { minute++; second = 0; }
            else        //小时+1
            { hour++; minute = 0; second = 0; }
        }
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    const QString organization = "CrystalEngraving";    //组织名
    const QString appName = "3DCurving";                //应用名

    // 注册表组-软件信息
    const QString g_SoftwareInfo = "si";
    // 运行时间和雕刻时间，注：短名隐晦其含义
    const QString sRunningTimeH = "rt1";    //运行时间，小时
    const QString sRunningTimeM = "rt2";    //运行时间，分钟
    const QString sRunningTimeS = "rt3";    //运行时间，秒
    const QString sEngravingTimeH = "et1";  //雕刻时间，小时
    const QString sEngravingTimeM = "et2";  //雕刻时间，分钟
    const QString sEngravingTimeS = "et3";  //雕刻时间，秒
    // 最近打开的文件、路径
    const QString sRecentFileList = "recentFileList";
    const QString sRecentPathList = "recentPathList";


    const osg::Vec3 engravingRange = osg::Vec3(500.f, 500.f, 300.f);    //雕刻幅面



private slots:

    /**   文件菜单    **/
    bool on_FileOpen(QString fileName = QString());
    void on_ClearAll();
    void on_OpenRecentFile();
    void on_OpenRecentPath();
//    void slot_FileSave();
    void on_quitApplication();

    /**   机器参数    **/
    void on_ParameterSetting();
//    void slot_LaserPara();
//    void slot_LaserRegImp();
//    void slot_LaserRegExp();
//    void slot_LaserBatchSet();
//    void slot_LaserTagGen();
//    void slot_LaserCal();
//    void slot_LaserPosTest();
//    void slot_LaserPlatSetH();

    /**   视图菜单    **/
    void on_setViewDirection(ViewDirection direction=DEFAULTVIEW);
//    void slot_ViewForth();
//    void slot_ViewTop();
//    void slot_ViewRight();
//    void slot_ViewUpdate();
//    void slot_ViewOriModel();
//    void slot_ViewModModel();
//    void slot_ViewApartModel();

//    /**   排序菜单    **/
//    void slot_SortY2X();
//    void slot_SortX2Y();
//    void slot_SortOri();
//    void slot_SortShort();

//    /**   分块菜单    **/
//    void slot_BlockX2Y();
//    void slot_BlockY2X();
//    void slot_BlockShort();
//    void slot_BlockPara();

//    /**   操作菜单    **/
//    void slot_OperLaserOri();
//    void slot_OperPlatHome();
//    void slot_OperSetCurPos2LaserOri();
//    void slot_OperSaveCurPos2LaserOri();

//    /**   调试菜单    **/
//    void slot_DebugAtchModel();
//    void slot_DebugHotTest();
//    void slot_DebugSplitModel();
//    void slot_DebugStopFun();
//    void slot_StdModCube();
//    void slot_StdModLine();
//    void slot_StdModPlat();
//    void slot_StdModSphere();

//    /**   帮助菜单    **/
//    void slot_HelpAboutMe();

//    /**   语言菜单    **/
//    void slot_LanCHN();

    /**   侧边栏    **/
    void on_StartEngraving();
    void on_PauseEngraving();
    void on_StopEngraving();
    void on_LaserOnOff(bool checked=false);

    void on_MoveToStartPos();
    void on_ResetPosition();
    void on_StopMoving();
    void on_MoveXP();
    void on_MoveXN();
    void on_MoveYP();
    void on_MoveYN();
    void on_MoveZP();
    void on_MoveZN();

    void on_OperationModeChanged();
    void on_Preview();
    void on_Apply();


    /**   计时器    **/
    void on_RunningTimer();         //运行时间
    void on_MotionMonitoringTimer();//运动监控

    void on_setAxesVisible(bool visible);

    // 私有方法
private:

    void resizeEvent(QResizeEvent *event);


    void initStatusBar();
    void initTimer();       //初始化运行时间计时器
    void endTimer();        //结束运行时间计时器
    void initSignalsAndSlots();
    void initActions();

    void initOSG();
    void initProjectionAsOrtho();
    void updateRefShape(osg::Geode *geode);
    void updateLighting(bool brightening);

    int initCtrlBoard();
    void inline axisMoveToDirection(MoveDirection dir);   // 轴移动
    void stopMove(int mode=DEC_STOP);
    void inline axisMoveTo(AXIS axis, float pos);
    bool motorIsRunning();

    // 处理最近打开文件及路径列表
    void saveRecentFileList(const QString &fileName, bool isRemove=false);
    void saveRecentPathList(const QString &path, bool isRemove=false);
    void updateRecentFileAndPathActions();


    osg::Geode *createCrystalFrame(osg::Vec3 size, float zRot);
    osg::BoundingBox getBoundingBox(osg::Node *node);
    osg::Vec3Array *getVertexArray(osg::Node *node);    //获取顶点/点云
    unsigned int getNumVertex(osg::Node *node);

    void addPointsToPointCloudGroup(osg::Group *pcGroup, osg::Vec3Array *points, bool removeOld=true);
    void updateStatusBar(QString statusMsg=QString());
    void updateDockWidget();        //更新侧边栏



    // UI窗体/控件
private:

    // actions
    QAction *recentFileActs[MAX_NUMFILES];
    QAction *recentPathActs[MAX_NUMFILES];

    // statusbar content
    QSpacerItem* hSpacer1;
    QLabel *lb_totalRunTime;
    QLabel *lb_totalEngravingTime;
    QTimer runningTimer;    //计时器
    QTimer motionMonitoringTimer;        //位置刷新计时器

    // 对话框
    MarkCodeWindow markCodeDialog;
    BatchEngravingDialog batchEngravingDialog;

private:

    Ui::MainWindow *ui;

    osg::ref_ptr<osgContainer> curViewer;
    osg::ref_ptr<osg::Switch> curRoot;
    osg::ref_ptr<osg::Geode> refShape;
    osg::ref_ptr<osg::Group> pointCloudGroup;


    meshInterface MI;   // 读取文件接口
    QDir lastUsedDirectory;


    bool bHomed = false;    //已经复位过
    bool bMotorRunning = false; //有电机在运行
    bool isLaserOn = false; //激光打开
    osg::Vec3 curPosition = osg::Vec3(0.f, 0.f, 0.f);
    TimerData tdRunningTime;
    TimerData tdEngravingTime;

    osg::ref_ptr<sharedParameter> sharedPara = new sharedParameter;
    CtrlCard ctrlCard;





};

#endif // MAINWINDOW_H
