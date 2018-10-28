#include "output_widget.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>

#include "semantic_error.hpp"

OutputWidget::OutputWidget() {
  
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
    
  if(!interp.parseStream(expression)){
    scene->addText("Error: Invalid Expression. Could not parse.");
  }
  else{
    try{
      Expression exp = interp.evaluate();
      result << exp;
      if (!exp.isLambda()) {
		    scene->addText(result.str().c_str());
      }
    }
    catch(const SemanticError & ex){
		  scene->addText(ex.what());
    }
  }
}