#include "output_widget.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <fstream>

#include "semantic_error.hpp"
#include "startup_config.hpp"

OutputWidget::OutputWidget(QWidget * parent): QWidget(parent){
  
  scene = new QGraphicsScene;
  view = new QGraphicsView(scene);

}

QWidget * OutputWidget::get_view(){
  return view;  
}

void OutputWidget::eval(std::string s) {
  std::istringstream expression(s);
  std::stringstream result;

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
        int y = exp.tailConstBegin()->head().asNumber();
        int radius =  exp.get_property(Atom("\"size\"")).head().asNumber();


        QGraphicsEllipseItem * point = scene->addEllipse(x, y, radius, radius);
        point->setBrush(Qt::black);
      }
      else if (exp.get_property(Atom("\"object-name\"")) == Expression(Atom("\"line\""))){

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