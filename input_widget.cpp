#include "input_widget.hpp"

InputWidget::InputWidget(QWidget * parent): QPlainTextEdit(parent){}

void InputWidget::keyPressEvent(QKeyEvent * event) {

  if (event->modifiers() == Qt::ShiftModifier && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)){
    emit input_recieved(this->toPlainText().toStdString());
  }
  else QPlainTextEdit::keyPressEvent(event);

}