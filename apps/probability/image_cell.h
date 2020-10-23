#ifndef PROBABILITY_IMAGE_CELL_H
#define PROBABILITY_IMAGE_CELL_H

namespace Probability {

class ImageCell : public Escher::HighlightCell {
public:
  ImageCell();
  void setHighlighted(bool highlight) override;
  void setImage(const Escher::Image * image, const Escher::Image * focusedImage);
  constexpr static KDCoordinate k_width = 39;
  constexpr static KDCoordinate k_height = 23;
private:
  int numberOfSubviews() const override;
  Escher::View * subviewAtIndex(int index) override;
  void layoutSubviews(bool force = false) override;
  Escher::ImageView m_iconView;
  const Escher::Image * m_icon;
  const Escher::Image * m_focusedIcon;
};

}

#endif
