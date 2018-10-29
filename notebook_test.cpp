#include <QTest>

#include "notebook_app.hpp"
#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>

class NotebookTest : public QObject {
  Q_OBJECT

private slots:

  void initTestCase();

  void testNoneProperty();
  void testCosine();
  void testExponential();
  void testDefineText();
  void testLambdaNoOutput();
  void testMakeZeroPoint();
  void testMakePoint();
  void testMultiplePoint();

private:

  NotebookApp notebook;

};

void NotebookTest::initTestCase(){

  auto in = notebook.findChild<InputWidget *>();
  QVERIFY2(in, "Could not find input widget");

  auto out = notebook.findChild<OutputWidget *>();
  QVERIFY2(out, "Could not find output widget");

}

void NotebookTest::testNoneProperty() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(get-property \"key\" (3))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("NONE"));

}

void NotebookTest::testCosine() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(cos pi)");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("(-1)"));

}

void NotebookTest::testExponential() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(^ e (- (* I pi)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("(-1,-1.22465e-16)"));

}

void NotebookTest::testDefineText() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(begin (define title \"The Title\") (title))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("(\"The Title\")"));

}

void NotebookTest::testLambdaNoOutput() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(define inc (lambda (x) (+ x 1)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();

  QVERIFY2(itemList.size() == 0, "Output present for user-defined function");

}

void NotebookTest::testMakeZeroPoint() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(make-point 0 0)");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsEllipseItem *>(itemList.front());

  QVERIFY2(item->rect() == QRectF(0, 0, 0, 0), "Point is not as expected");

}

void NotebookTest::testMakePoint() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(set-property \"size\" 20 (make-point 0 0))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsEllipseItem *>(itemList.front());

  QVERIFY2(item->rect() == QRectF(-10, -10, 20, 20), "Point is not as expected");

}

void NotebookTest::testMultiplePoint() {

  auto in = notebook.findChild<InputWidget *>();
  auto out = notebook.findChild<OutputWidget *>();

  in->clear();

  QTest::keyClicks(in, "(list (set-property \"size\" 14 (make-point 7 9))\
                        (set-property \"size\" 5 (make-point 3 2)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto firstPoint = dynamic_cast<QGraphicsEllipseItem *>(itemList[1]);
  auto secondPoint = dynamic_cast<QGraphicsEllipseItem *>(itemList[0]);

  QVERIFY2(firstPoint->rect() == QRectF(0, 2, 14, 14), "First point is not as expected");
  QVERIFY2(secondPoint->rect() == QRectF(0.5, -0.5, 5, 5), "Second point is not as expected");
  
}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
