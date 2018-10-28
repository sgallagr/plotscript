#ifndef INPUT_WIDGET_H
#define INPUT_WIDGET_H

#include <QPlainTextEdit>
#include <QKeyEvent>

class InputWidget: public QPlainTextEdit{
Q_OBJECT

public:

  InputWidget(QWidget * parent = nullptr);

  void keyPressEvent(QKeyEvent * event);

signals:

  void input_recieved(std::string s);

  
};

#endif