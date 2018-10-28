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

}

QWidget * OutputWidget::get_view(){
  return view;  
}

void OutputWidget::eval(std::string s) {
  std::istringstream expression(s);
  std::stringstream result;

  Interpreter interp;

  scene->clear();

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
    
  if(!interp.parseStream(expression)){
    scene->addText("Error: Invalid Expression. Could not parse.");
  }
  else{
    try{
      Expression exp = interp.evaluate();
      result << exp;
      if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"point\""))){
        int x = exp.tailConstBegin()->head().asNumber();
        int y = (exp.tailConstEnd() - 1)->head().asNumber();
        int radius =  exp.get_property(Atom("\"size\"")).head().asNumber() / 2;

        if (!(radius < 0)){
          QGraphicsEllipseItem * point = scene->addEllipse(x, y, radius, radius);
          point->setBrush(Qt::black);
        }
        else scene->addText("Error: Point size is not a positive number.");
      }
      else if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"line\""))){
        int x1 = exp.tailConstBegin()->tailConstBegin()->head().asNumber();
        int y1 = (exp.tailConstBegin()->tailConstEnd() - 1)->head().asNumber();
        int x2 = (exp.tailConstEnd() - 1)->tailConstBegin()->head().asNumber();
        int y2 = ((exp.tailConstEnd() - 1)->tailConstEnd() - 1)->head().asNumber();
        int width =  exp.get_property(Atom("\"thickness\"")).head().asNumber();

        if (!(width < 0)){
          QGraphicsLineItem * line = scene->addLine(x1, y1, x2, y2);
          QPen pen;
          pen.setWidth(width);
          line->setPen(pen);
        }
        else scene->addText("Error: Line thickness is not a positive number.");

      }
      else if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"text\""))){
        Expression pos_prop = exp.get_property(Atom("\"position\""));
        QGraphicsTextItem * text;

        if (pos_prop.get_property(Atom("\"object-name\"")) == Expression(Atom("\"point\""))){
          int x = pos_prop.tailConstBegin()->head().asNumber();
          int y = (pos_prop.tailConstEnd() - 1)->head().asNumber();

          text = scene->addText(exp.head().asSymbol().c_str());
          text->setPos(x, y);
        }
        else scene->addText("Error: Invalid position property");
      }
      else if (!exp.isLambda()) {
		    scene->addText(result.str().c_str());
      }
      
    }
    catch(const SemanticError & ex){
		  scene->addText(ex.what());
    }
  }
}