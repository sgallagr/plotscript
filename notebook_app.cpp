#include "notebook_app.hpp"

#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QVBoxLayout>

NotebookApp::NotebookApp() {
  
  InputWidget *input = new InputWidget(this);
  OutputWidget *output = new OutputWidget(this);

  input->setObjectName("input");
  output->setObjectName("output");

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(input);
  layout->addWidget(output->get_view());
  
  setLayout(layout);

  QObject::connect(input, &InputWidget::input_recieved, output, &OutputWidget::eval);

}