#include "output_widget.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <fstream>
#include <QGridLayout>

#include "semantic_error.hpp"

OutputWidget::OutputWidget(QWidget * parent) : QWidget(parent) {

  scene = new QGraphicsScene;
  view = new QGraphicsView(scene);

  view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto layout = new QGridLayout;
  layout->addWidget(view, 0, 0);

  setLayout(layout);

  interp = Interpreter(&program_queue, &expression_queue);

  interp_th = std::thread(&interp);

}

void OutputWidget::resizeEvent(QResizeEvent * event) {
  this->QWidget::resizeEvent(event);
  view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void OutputWidget::handle_point(Expression & exp) {
  double x = exp.tailConstBegin()->head().asNumber();
  double y = (exp.tailConstEnd() - 1)->head().asNumber();
  double diameter = exp.get_property(Atom("\"size\"")).head().asNumber();
  double radius = diameter / 2;

  double x_center = x - radius;
  double y_center = y - radius;
  double width = diameter;
  double height = diameter;

  if (!(diameter < 0.)) {
    QGraphicsEllipseItem * point = scene->addEllipse(x_center, y_center, width, height);
    point->setBrush(Qt::black);
    QPen pen;
    pen.setWidth(0);
    pen.setBrush(Qt::black);
    point->setPen(pen);
  }
  else scene->addText("Error: Point size is not a positive number.");
 }

void OutputWidget::handle_line(Expression & exp) {
  double x1 = exp.tailConstBegin()->tailConstBegin()->head().asNumber();
  double y1 = (exp.tailConstBegin()->tailConstEnd() - 1)->head().asNumber();
  double x2 = (exp.tailConstEnd() - 1)->tailConstBegin()->head().asNumber();
  double y2 = ((exp.tailConstEnd() - 1)->tailConstEnd() - 1)->head().asNumber();
  double width = exp.get_property(Atom("\"thickness\"")).head().asNumber();

  if (!(width < 0.)) {
    QGraphicsLineItem * line = scene->addLine(x1, y1, x2, y2);
    QPen pen;
    pen.setWidth(width);
    line->setPen(pen);
  }
  else scene->addText("Error: Line thickness is not a positive number.");
}

void OutputWidget::handle_text(Expression & exp) {
  Expression pos_prop = exp.get_property(Atom("\"position\""));
  QGraphicsTextItem * text;

  if (pos_prop.get_property(Atom("\"object-name\"")) == Expression(Atom("\"point\""))) {
    double x = pos_prop.tailConstBegin()->head().asNumber();
    double y = (pos_prop.tailConstEnd() - 1)->head().asNumber();
    double height, width;
    const double PI = std::atan2(0, -1);
    double scale_val;
    double rotate_val;

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(1);

    std::string str = exp.head().asSymbol();

    // remove quotations at beginning and end
    if (str.front() == '"') {
      str.replace(str.begin(), str.begin() + 1, "");
      str.replace(str.end() - 1, str.end(), "");
    }

    if (exp.get_property(Atom("\"text-rotation\"")).head().isNumber()){
      rotate_val = (180/PI) * exp.get_property(Atom("\"text-rotation\"")).head().asNumber();
    }
    else {
      rotate_val = 0;
    }

    if (exp.get_property(Atom("\"text-scale\"")).head().isNumber()){
      scale_val = exp.get_property(Atom("\"text-scale\"")).head().asNumber();
    }
    else {
      scale_val = 1;
    }

    text = scene->addText(str.c_str());
    text->setFont(font);
    text->setScale(scale_val);
    height = text->boundingRect().height();
    width = text->boundingRect().width();
    text->setTransformOriginPoint(QPointF(width/2, height/2));
    text->setPos(x - (width/2), y - (height/2));
    text->setRotation(rotate_val);
  }
  else scene->addText("Error: Invalid position property");
}

void OutputWidget::process(Expression exp) {
  std::stringstream result;

  if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"point\""))){
    handle_point(exp);
  }
  else if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"line\""))){
    handle_line(exp);
  }
  else if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"text\""))){
    handle_text(exp);
  }
  else if (exp.isList()) {
    for(auto it = exp.tailConstBegin(); it != exp.tailConstEnd(); ++it){
       process(*it);
    }
  }
  else if (!exp.isLambda()) {
    result << exp;
		scene->addText(result.str().c_str());
  }

  view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void OutputWidget::eval(std::string s) {
  /*std::istringstream expression(s);
  
  scene->clear();
    
  if(!interp.parseStream(expression)){
    scene->addText("Error: Invalid Expression. Could not parse.");
  }
  else{
    try{
      process(interp.evaluate());
    }
    catch(const SemanticError & ex){
		  scene->addText(ex.what());
    }
  }

  view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);*/
  
  
  
  std::string input = s;

  if (running) {
    if (input.front() == '%') {
      if(input == "%start") {}
      else if(input == "%stop") {
        program_queue.push(input);
        running = 0;
      }
      else if (input == "%reset") {
        program_queue.push("%stop");
        interp_th.join();
        interp = Interpreter(&program_queue, &expression_queue);
        interp_th = std::thread(interp);
      }
      else {
        scene->clear();
        scene->addText("Error: invalid kernel directive");
        view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
      }
    }
    else {
      program_queue.push(input);

      Expression exp;
      expression_queue.wait_and_pop(exp);

      scene->clear();
      if(exp.head().asSymbol() == "Error") scene->addText(exp.tailConstBegin()->head().asSymbol().c_str());
      else process(exp);
      view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        
    }
  }
  else {
    if (input.front() == '%') {
      if(input == "%start") {
        running = 1;
        interp_th.join();
        interp_th = std::thread(interp);
      }
      else if(input == "%stop") {}
      else if (input == "%reset") {
        running = 1;
        interp_th.join();
        interp = Interpreter(&program_queue, &expression_queue);
        interp_th = std::thread(interp);
      }
      else {
        scene->clear();
        scene->addText("Error: invalid kernel directive");
        view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
      }
    }
    else {
      scene->clear();
      scene->addText("Error: interpreter kernel not running");
      view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
  }
}

void OutputWidget::start_kernel() {
  eval("%start");
}

void OutputWidget::stop_kernel() {
  eval("%stop");
}

void OutputWidget::reset_kernel() {
  eval("%reset");
}

void OutputWidget::interrupt() {}