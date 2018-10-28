#include "notebook_app.hpp"

#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QVBoxLayout>

NotebookApp::NotebookApp(QWidget * parent): QWidget(parent){
  
  InputWidget *input = new InputWidget(parent);
  OutputWidget *output = new OutputWidget(parent);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(input);
  layout->addWidget(output->get_view());
  
  setLayout(layout);

  QObject::connect(input, &InputWidget::input_recieved, output, &OutputWidget::eval);

}