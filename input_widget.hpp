#ifndef INPUT_WIDGET_H
#define INPUT_WIDGET_H

#include <QPlainTextEdit>

class InputWidget: public QPlainTextEdit{
Q_OBJECT

public:

  InputWidget(QPlainTextEdit * parent = nullptr);

  
};

#endif