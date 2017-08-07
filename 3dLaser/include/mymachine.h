#ifndef MYMACHINE_H
#define MYMACHINE_H

#include <QThread>

class MotorMotion : public QThread
{
public:
    MotorMotion();

protected:
    void run();
};

#endif // MYMACHINE_H
