#ifndef OUTPUT_WIDGET_H
#define OUTPUT_WIDGET_H

#include <QWidget>

#include "interpreter.hpp"

class QGraphicsView;
class QGraphicsScene;

class OutputWidget: public QWidget{
Q_OBJECT

public:

  OutputWidget(QWidget * parent = nullptr);

  QWidget * get_view();

public slots:

  void eval(std::string s);

private:

  QGraphicsScene * scene;
	QGraphicsView * view;

  Interpreter interp;
};

#endif