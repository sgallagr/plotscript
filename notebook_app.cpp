#include "notebook_app.hpp"

#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

NotebookApp::NotebookApp() {
  
  auto input = new InputWidget(this);
  auto output = new OutputWidget(this);
  auto buttons = new QWidget(this);

  input->setObjectName("input");
  output->setObjectName("output");

  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *button_layout = new QHBoxLayout;

  auto start = new QPushButton("Start Kernel");
  button_layout->addWidget(start);
  auto stop = new QPushButton("Stop Kernel");
  button_layout->addWidget(stop);
  auto reset = new QPushButton("Reset Kernel");
  button_layout->addWidget(reset);
  auto interrupt = new QPushButton("Interrupt");
  button_layout->addWidget(interrupt);

  buttons->setLayout(button_layout);

  layout->addWidget(buttons);
  layout->addWidget(input);
  layout->addWidget(output);
  
  setLayout(layout);

  QObject::connect(input, &InputWidget::input_recieved, output, &OutputWidget::eval);
  QObject::connect(start, SIGNAL(clicked()), output, SLOT(start_kernel()));
  QObject::connect(stop, SIGNAL(clicked()), output, SLOT(stop_kernel()));
  QObject::connect(reset, SIGNAL(clicked()), output, SLOT(reset_kernel()));
  QObject::connect(interrupt, SIGNAL(clicked()), output, SLOT(interrupt()));
}