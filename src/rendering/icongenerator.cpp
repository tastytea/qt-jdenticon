#include "icongenerator.h"

#include <QCryptographicHash>
namespace rendering {

QList<QPoint> IconGenerator::shapeOne_({QPoint(1, 0),
                                        QPoint(2, 0),
                                        QPoint(2, 3),
                                        QPoint(1, 3),
                                        QPoint(0, 1),
                                        QPoint(3, 1),
                                        QPoint(3, 2),
                                        QPoint(0, 2)});
QList<QPoint> IconGenerator::shapeTwo_({{0, 0}, {3, 0}, {3, 3}, {0, 3}});
QList<QPoint> IconGenerator::shapeThree_({{1, 1}, {2, 1}, {2, 2}, {1, 2}});
QList<void (*)(rendering::Renderer &, int, int)> IconGenerator::centerShapes_ =
  shapes::ShapeDefinitions::CenterShapes();
QList<void (*)(rendering::Renderer &, int, int)> IconGenerator::outerShapes_ =
  shapes::ShapeDefinitions::OuterShapes();

QList<shapes::ShapeCategory> IconGenerator::defaultCategories_ = {
  // Sides
  shapes::ShapeCategory(8, 2, 3, shapeOne_, outerShapes_),
  // Corner
  shapes::ShapeCategory(9, 4, 5, shapeTwo_, outerShapes_),
  // Center
  shapes::ShapeCategory(10, 1, -1, shapeThree_, centerShapes_)};

IconGenerator::IconGenerator() {}

QList<shapes::ShapeCategory>
IconGenerator::getCategories()
{
    return defaultCategories_;
}

QList<shapes::Shape>
IconGenerator::getShapes(ColorTheme &theme, QString &hash)
{
    QList<shapes::Shape> shapes;
    QList<int> usedColorThemeIndexes;

    for (auto category : getCategories()) {
        qDebug() << "themeCount " << theme.count();
        auto colorThemeIndex = getOctet(hash, category.getColorIndex()) % theme.count();

        QList<int> dupsOne = {0, 4};
        QList<int> dupsTwo = {2, 3};
        if (isDuplicate(usedColorThemeIndexes,
                        colorThemeIndex,
                        dupsOne) || // Disallow dark gray and dark color combo
            isDuplicate(usedColorThemeIndexes,
                        colorThemeIndex,
                        dupsTwo)) // Disallow light gray and light color combo
        {
            colorThemeIndex = 1;
        }

        usedColorThemeIndexes.append(colorThemeIndex);

        auto startRotationIndex =
          category.getRotationIndex() == -1 ? 0 : getOctet(hash, category.getRotationIndex());
        auto octet          = getOctet(hash, category.getShapeIndex());
        auto definitionSize = category.getDefinitions().size();
        qDebug() << "definitionSize " << definitionSize;
        auto shape = category.getDefinitions()[octet % definitionSize];
        qDebug() << "Shape # " << (octet % definitionSize);
        auto positions         = category.getPositions();
        shapes::Shape newShape = {shape, theme[colorThemeIndex], positions, startRotationIndex};
        shapes.append(newShape);
    }
    return shapes;
}

void
IconGenerator::generate(Renderer &renderer, Rectangle &rect, IdenticonStyle &style, QString &input)
{
    auto hue = getHue(input);

    qDebug() << "hue" << hue;
    auto colorTheme = ColorTheme(hue, style);
    QString hash =
      QString(QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha1).toHex());

    RenderBackground(renderer, rect, style, colorTheme, hash);
    RenderForeground(renderer, rect, style, colorTheme, hash);
}

uint32_t
IconGenerator::hashQString(const QString &input)
{
    auto h = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha1);

    return (h[0] << 24) ^ (h[1] << 16) ^ (h[2] << 8) ^ h[3];
}
} // namespace rendering
