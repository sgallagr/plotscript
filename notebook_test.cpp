#include <QTest>

#include "notebook_app.hpp"
#include "input_widget.hpp"
#include "output_widget.hpp"

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QMarginsF>

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
  void testDiscretePlot();
  void testContinuousPlot();

private:

  NotebookApp notebook;

  InputWidget * in = nullptr;
  OutputWidget * out = nullptr;

  int findLines(QGraphicsScene * scene, QRectF bbox, qreal margin);
  int findPoints(QGraphicsScene * scene, QPointF center, qreal radius);
  int findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents);
  int intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius);

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
                        (set-property \"position\" (make-point (+ xloc 20) yloc) (make-text \"Hi1\"))\
                        (set-property \"position\" (make-point (+ xloc 40) yloc) (make-text \"Hi2\"))\
                        (set-property \"position\" (make-point (+ xloc 60) yloc) (make-text \"Hi3\"))\
                        (set-property \"position\" (make-point (+ xloc 80) yloc) (make-text \"Hi4\"))\
                        (set-property \"position\" (make-point (+ xloc 100) yloc) (make-text \"Hi5\"))))");

  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto itemList = out->scene->items();
  auto firstText = dynamic_cast<QGraphicsTextItem *>(itemList[4]);
  auto secondText = dynamic_cast<QGraphicsTextItem *>(itemList[3]);
  auto thirdText = dynamic_cast<QGraphicsTextItem *>(itemList[2]);
  auto fourthText = dynamic_cast<QGraphicsTextItem *>(itemList[1]);
  auto fifthText = dynamic_cast<QGraphicsTextItem *>(itemList[0]);

  QCOMPARE(QString(firstText->toPlainText()), QString("Hi1"));

  QCOMPARE(QString(secondText->toPlainText()), QString("Hi2"));

  QCOMPARE(QString(thirdText->toPlainText()), QString("Hi3"));

  QCOMPARE(QString(fourthText->toPlainText()), QString("Hi4"));

  QCOMPARE(QString(fifthText->toPlainText()), QString("Hi5"));

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

/* 
findLines - find lines in a scene contained within a bounding box 
            with a small margin
 */
int NotebookTest::findLines(QGraphicsScene * scene, QRectF bbox, qreal margin){

  QPainterPath selectPath;

  QMarginsF margins(margin, margin, margin, margin);
  selectPath.addRect(bbox.marginsAdded(margins));
  scene->setSelectionArea(selectPath, Qt::ContainsItemShape);
  
  int numlines(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsLineItem::Type){
      numlines += 1;
    }
  }

  return numlines;
}

/* 
findPoints - find points in a scene contained within a specified rectangle
 */
int NotebookTest::findPoints(QGraphicsScene * scene, QPointF center, qreal radius){
  
  QPainterPath selectPath;
  selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
  scene->setSelectionArea(selectPath, Qt::ContainsItemShape);

  int numpoints(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsEllipseItem::Type){
      numpoints += 1;
    }
  }

  return numpoints;
}

/* 
findText - find text in a scene centered at a specified point with a given 
           rotation and string contents  
 */
int NotebookTest::findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents){
  
  int numtext(0);
  foreach(auto item, scene->items(center)){
    if(item->type() == QGraphicsTextItem::Type){
      QGraphicsTextItem * text = static_cast<QGraphicsTextItem *>(item);
      if((text->toPlainText() == contents) &&
     (text->rotation() == rotation) &&
     (text->pos() + text->boundingRect().center() == center)){
    numtext += 1;
      }
    }
  }

  return numtext;
}

/* 
intersectsLine - find lines in a scene that intersect a specified rectangle
 */
int NotebookTest::intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius){
              
  QPainterPath selectPath;
  selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
  scene->setSelectionArea(selectPath, Qt::IntersectsItemShape);

  int numlines(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsLineItem::Type){
      numlines += 1;
    }
  }

  return numlines;
}

void NotebookTest::testDiscretePlot() {

  in->clear();

  QTest::keyClicks(in, "(discrete-plot (list (list -1 -1) (list 1 1))\
                        (list (list \"title\" \"The Title\")\
                        (list \"abscissa-label\" \"X Label\")\
                        (list \"ordinate-label\" \"Y Label\") ))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto scene = out->scene;

  // first check total number of items
  // 8 lines + 2 points + 7 text = 17
  auto itemList = scene->items();
  QCOMPARE(itemList.size(), 17);

  // make them all selectable
  foreach(auto item, itemList){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }

  double scalex = 20.0/2.0;
  double scaley = 20.0/2.0;

  double xmin = scalex*-1;
  double xmax = scalex*1;
  double ymin = scaley*-1;
  double ymax = scaley*1;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;
    
  // check title
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("The Title")), 1);
  
  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("X Label")), 1);
  
  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("Y Label")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-1")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("1")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-1")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("1")), 1);

  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 2);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 2);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis 
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);
  
  // check the point at (-1,-1)
  QCOMPARE(findPoints(scene, QPointF(-10, 10), 0.6), 1);
    
  // check the point at (1,1)
  QCOMPARE(findPoints(scene, QPointF(10, -10), 0.6), 1); 
}

void NotebookTest::testContinuousPlot() {

  in->clear();

  QTest::keyClicks(in, "(begin (define f (lambda (x) (+ (* 2 x) 1)))\
                        (continuous-plot f (list -2 2)\
                        (list (list \"title\" \"A continuous linear function\")\
                        (list \"abscissa-label\" \"x\")\
                        (list \"ordinate-label\" \"y\") )))");
  QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 4);

  auto scene = out->scene;

  // first check total number of items
  // 56 lines + 7 text = 63
  auto itemList = scene->items();
  QCOMPARE(itemList.size(), 63);

  // make them all selectable
  foreach(auto item, itemList){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }

  double scalex = 20.0/4.0;
  double scaley = 20.0/8.0;

  double xmin = scalex*-2;
  double xmax = scalex*2;
  double ymin = scaley*-3;
  double ymax = scaley*5;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;
    
  // check title
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("A continuous linear function")), 1);
  
  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("x")), 1);
  
  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("y")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-2")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("2")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-3")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("5")), 1);

  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 1);

  // check the bounding box right
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 1);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis 
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);
}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
