#ifndef OUTPUT_WIDGET_H
#define OUTPUT_WIDGET_H

#include <QWidget>
#include <QGraphicsTextItem>

#include "interpreter.hpp"

class QGraphicsView;
class QGraphicsScene;

class OutputWidget: public QWidget{
Q_OBJECT

public:

  OutputWidget(QWidget * parent = nullptr);

  void startup();

  void resizeEvent(QResizeEvent * event);

  void process(Expression e);
  void handle_point(Expression & exp);
  void handle_line(Expression & exp);
  void center_text(QGraphicsTextItem & text, double x, double y);
  void handle_text(Expression & exp);

  QGraphicsScene * scene;
	QGraphicsView * view;

public slots:

  void eval(std::string s);

private:

  Interpreter interp;

};

#endif