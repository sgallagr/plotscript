#include "notebook_app.hpp"

#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QVBoxLayout>

NotebookApp::NotebookApp(QWidget * parent): QWidget(parent){
  
  InputWidget *input = new InputWidget;
  OutputWidget *output = new OutputWidget;

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(input);
  layout->addWidget(output);
  
  setLayout(layout);

  

}