#ifndef OUTPUT_WIDGET_H
#define OUTPUT_WIDGET_H

#include <QWidget>
#include <QGraphicsTextItem>
#include <thread>

#include "interpreter.hpp"

class QGraphicsView;
class QGraphicsScene;

class OutputWidget: public QWidget{
Q_OBJECT

public:

  OutputWidget(QWidget * parent = nullptr);

  void resizeEvent(QResizeEvent * event);

  void process(Expression exp);
  void handle_point(Expression & exp);
  void handle_line(Expression & exp);
  void handle_text(Expression & exp);

  QGraphicsScene * scene;
	QGraphicsView * view;

public slots:

  void eval(std::string s);

  /*void start_kernel();
  void stop_kernel();
  void reset_kernel();
  void interrupt();*/

private:

  ThreadSafeQueue<std::string> program_queue;
  ThreadSafeQueue<Expression> expression_queue;
  int running = 1;

  Interpreter interp;
  std::thread interp_th;

};

#endif