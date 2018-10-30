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
  void testMakeLine();
  void testMultipleLine();
  void testMakeText();
  void testMultipleText();
  void testInvalidExpression();
  void testSemanticError();

private:

  NotebookApp notebook;

  InputWidget * in = nullptr;
  OutputWidget * out = nullptr;

};

void NotebookTest::initTestCase(){

  in = notebook.findChild<InputWidget *>();
  QVERIFY2(in, "Could not find input widget");

  out = notebook.findChild<OutputWidget *>();
  QVERIFY2(out, "Could not find output widget");

}

void NotebookTest::testNoneProperty() {

  in->clear();

  QTest::keyClicks(in, "(get-property \"key\" (3))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("NONE"));

}

void NotebookTest::testCosine() {

  in->clear();

  QTest::keyClicks(in, "(cos pi)");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("(-1)"));

}

void NotebookTest::testExponential() {

  in->clear();

  QTest::keyClicks(in, "(^ e (- (* I pi)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("(-1,-1.22465e-16)"));

}

void NotebookTest::testDefineText() {

  in->clear();

  QTest::keyClicks(in, "(begin (define title \"The Title\") (title))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("(\"The Title\")"));

}

void NotebookTest::testLambdaNoOutput() {

  in->clear();

  QTest::keyClicks(in, "(define inc (lambda (x) (+ x 1)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();

  QCOMPARE(itemList.size(), 0);

}

void NotebookTest::testMakeZeroPoint() {

  in->clear();

  QTest::keyClicks(in, "(make-point 0 0)");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsEllipseItem *>(itemList.front());

  QCOMPARE(item->rect(), QRectF(0, 0, 0, 0));

}

void NotebookTest::testMakePoint() {

  in->clear();

  QTest::keyClicks(in, "(set-property \"size\" 20 (make-point 0 0))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsEllipseItem *>(itemList.front());

  QCOMPARE(item->rect(), QRectF(-10, -10, 20, 20));

}

void NotebookTest::testMultiplePoint() {

  in->clear();

  QTest::keyClicks(in, "(list (set-property \"size\" 14 (make-point 7 9))\
                        (set-property \"size\" 5 (make-point 3 2)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto firstPoint = dynamic_cast<QGraphicsEllipseItem *>(itemList[1]);
  auto secondPoint = dynamic_cast<QGraphicsEllipseItem *>(itemList[0]);

  QCOMPARE(firstPoint->rect(), QRectF(0, 2, 14, 14));
  QCOMPARE(secondPoint->rect(), QRectF(0.5, -0.5, 5, 5));
  
}

void NotebookTest::testMakeLine() {

  in->clear();

  QTest::keyClicks(in, "(set-property \"thickness\" (4) (make-line\
                        (make-point 0 0) (make-point 20 20)))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsLineItem *>(itemList.front());

  QCOMPARE(item->line(), QLineF(0, 0, 20, 20));
  QCOMPARE(item->pen().width(), 4);
  
}

void NotebookTest::testMultipleLine() {

  in->clear();

  QTest::keyClicks(in, "(list (make-line (make-point 0 0) (make-point 0 20))\
                              (make-line (make-point 10 0) (make-point 10 20))\
                              (make-line (make-point 20 0) (make-point 20 20)))");

  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto firstLine = dynamic_cast<QGraphicsLineItem *>(itemList[2]);
  auto secondLine = dynamic_cast<QGraphicsLineItem *>(itemList[1]);
  auto thirdLine = dynamic_cast<QGraphicsLineItem *>(itemList[0]);

  QCOMPARE(firstLine->line(), QLineF(0, 0, 0, 20));
  QCOMPARE(firstLine->pen().width(), 1);

  QCOMPARE(secondLine->line(), QLineF(10, 0, 10, 20));
  QCOMPARE(secondLine->pen().width(), 1);

  QCOMPARE(thirdLine->line(), QLineF(20, 0, 20, 20));
  QCOMPARE(thirdLine->pen().width(), 1);
  
}

void NotebookTest::testMakeText() {

  in->clear();

  QTest::keyClicks(in, "(make-text \"Hello World!\")");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("Hello World!"));

}

void NotebookTest::testMultipleText() {

  in->clear();

  QTest::keyClicks(in, "(begin (define xloc 0) (define yloc 0) (list\
                        (set-property \"position\" (make-point (+ xloc 20) yloc) (make-text \"Hi\"))\
                        (set-property \"position\" (make-point (+ xloc 40) yloc) (make-text \"Hi\"))\
                        (set-property \"position\" (make-point (+ xloc 60) yloc) (make-text \"Hi\"))\
                        (set-property \"position\" (make-point (+ xloc 80) yloc) (make-text \"Hi\"))\
                        (set-property \"position\" (make-point (+ xloc 100) yloc) (make-text \"Hi\"))))");

  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto firstText = dynamic_cast<QGraphicsTextItem *>(itemList[4]);
  auto secondText = dynamic_cast<QGraphicsTextItem *>(itemList[3]);
  auto thirdText = dynamic_cast<QGraphicsTextItem *>(itemList[2]);
  auto fourthText = dynamic_cast<QGraphicsTextItem *>(itemList[1]);
  auto fifthText = dynamic_cast<QGraphicsTextItem *>(itemList[0]);

  QCOMPARE(QString(firstText->toPlainText()), QString("Hi"));
  QCOMPARE(firstText->pos(), QPointF(20, 0));

  QCOMPARE(QString(secondText->toPlainText()), QString("Hi"));
  QCOMPARE(secondText->pos(), QPointF(40, 0));

  QCOMPARE(QString(thirdText->toPlainText()), QString("Hi"));
  QCOMPARE(thirdText->pos(), QPointF(60, 0));

  QCOMPARE(QString(fourthText->toPlainText()), QString("Hi"));
  QCOMPARE(fourthText->pos(), QPointF(80, 0));

  QCOMPARE(QString(fifthText->toPlainText()), QString("Hi"));
  QCOMPARE(fifthText->pos(), QPointF(100, 0));

}

void NotebookTest::testInvalidExpression() {

  in->clear();

  QTest::keyClicks(in, "(begin))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("Error: Invalid Expression. Could not parse."));

}

void NotebookTest::testSemanticError() {

  in->clear();

  QTest::keyClicks(in, "(begin (define a I) (first a))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto item = dynamic_cast<QGraphicsTextItem *>(itemList.front());

  QCOMPARE(QString(item->toPlainText()), QString("Error in call to first: argument not a list"));

}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
