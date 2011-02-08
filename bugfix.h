#ifndef _bugfix_h_
#define _bugfix_h_

// work around a Qt QRect::normalized bug (Qt-4.7.1)
// returning 0 width/height for QRect(QPoint(1,1), QPoint(0,0)).normalized()

class bugfix_QRect : public QRect
{
  public:
    bugfix_QRect(const QPoint &p1, const QPoint &p2)
      : QRect(p1, p2)
    {
    }

    QRect normalized() const
    {
      int x1, x2, y1, y2;
      getCoords(&x1, &y1, &x2, &y2);

      int r_x1, r_x2, r_y1, r_y2;

      QRect r;
      if (x2 < x1) {                                // swap bad x values
          r_x1 = x2;
          r_x2 = x1;
      } else {
          r_x1 = x1;
          r_x2 = x2;
      }
      if (y2 < y1) {                                // swap bad y values
          r_y1 = y2;
          r_y2 = y1;
      } else {
          r_y1 = y1;
          r_y2 = y2;
      }
      r.setCoords(r_x1, r_y1, r_x2, r_y2);
      return r;
    }
};

#endif
