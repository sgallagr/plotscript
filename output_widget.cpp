#include "output_widget.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <fstream>
#include <QGridLayout>

#include "semantic_error.hpp"
#include "startup_config.hpp"

OutputWidget::OutputWidget(QWidget * parent): QWidget(parent){
  
  scene = new QGraphicsScene;
  view = new QGraphicsView(scene);

  auto layout = new QGridLayout;
  layout->addWidget(view, 0, 0);

  setLayout(layout);

  startup();

}

void OutputWidget::startup() {
  std::ifstream ifs(STARTUP_FILE);

  if(!ifs){
    scene->addText("Could not open startup file for reading.");
  }

  if(!interp.parseStream(ifs)){
    scene->addText("Invalid startup program. Could not parse.");
  }
  else{
    try{
      interp.evaluate();
    }
    catch(const SemanticError & ex){
      scene->addText("Startup program failed during evaluation");
    }	
  }
}

void OutputWidget::handle_point(Expression & exp) {
  double x = exp.tailConstBegin()->head().asNumber();
  double y = (exp.tailConstEnd() - 1)->head().asNumber();
  double radius =  exp.get_property(Atom("\"size\"")).head().asNumber() / 2;

  if (!(radius < 0.)){
    QGraphicsEllipseItem * point = scene->addEllipse(x, y, radius, radius);
    point->setBrush(Qt::black);
  }
  else scene->addText("Error: Point size is not a positive number.");
}

void OutputWidget::handle_line(Expression & exp) {
  double x1 = exp.tailConstBegin()->tailConstBegin()->head().asNumber();
  double y1 = (exp.tailConstBegin()->tailConstEnd() - 1)->head().asNumber();
  double x2 = (exp.tailConstEnd() - 1)->tailConstBegin()->head().asNumber();
  double y2 = ((exp.tailConstEnd() - 1)->tailConstEnd() - 1)->head().asNumber();
  double width =  exp.get_property(Atom("\"thickness\"")).head().asNumber();

  if (!(width < 0.)){
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

  if (pos_prop.get_property(Atom("\"object-name\"")) == Expression(Atom("\"point\""))){
    double x = pos_prop.tailConstBegin()->head().asNumber();
    double y = (pos_prop.tailConstEnd() - 1)->head().asNumber();

    std::string str = exp.head().asSymbol();

    // remove quotations at beginning and end
    str.replace(str.begin(), str.begin() + 1, "");
    str.replace(str.end() - 1, str.end(), "");

    text = scene->addText(str.c_str());
    text->setPos(x, y);
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
    
}

void OutputWidget::eval(std::string s) {
  std::istringstream expression(s);

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
}