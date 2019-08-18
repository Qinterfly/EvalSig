#ifndef LEVELSWINDOW_H
#define LEVELSWINDOW_H

#include <QDialog>

namespace Ui {
class LevelsWindow;
}

class LevelsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LevelsWindow(QWidget *parent = nullptr);
    ~LevelsWindow();

private:
    Ui::LevelsWindow *ui;
};

#endif // LEVELSWINDOW_H
