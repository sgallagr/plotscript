#include <QTest>

#include "notebook_app.hpp"
#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QGraphicsView>
#include <QGraphicsItem>

class NotebookTest : public QObject {
  Q_OBJECT

private slots:

  void initTestCase();

  void testNoneProperty();

private:

  NotebookApp widget;

};

void NotebookTest::initTestCase(){

  auto in = widget.findChild<InputWidget *>();
  QVERIFY2(in, "Could not find input widget");

  auto out = widget.findChild<OutputWidget *>();
  QVERIFY2(out, "Could not find output widget");

  auto view = out->findChild<QGraphicsView *>();
  QVERIFY2(view, "Could not find output view widget");

}

void NotebookTest::testNoneProperty() {

  auto in = widget.findChild<InputWidget *>();
  auto out = widget.findChild<OutputWidget *>();
  auto view = out->findChild<QGraphicsView *>();

  QTest::keyClicks(in, "(get-property \"key\" (3))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier);
  auto text = view->itemAt(QPoint(0,0));

  QVERIFY2(text, "Error in none property test: QGraphicsTextItem does not exist");

}


QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
