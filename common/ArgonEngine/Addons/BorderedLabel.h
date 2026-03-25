#ifndef BORDERED_LABEL
#define BORDERED_LABEL

#include <ArgonEngine/Text.h>
#include <ArgonEngine/Sprite.h>

namespace Argon{
namespace Addons{

/*! 
    \brief A label, with a sprite behind it.

    The BorderedLabel struct is a wrapper around Argon::Label and Argon::Sprite
    that makes sure the positions of the Label and the Sprite line up.
*/
struct BorderedLabel : public Label, public Sprite {

    BorderedLabel();

    /*!
        \brief generates the texture
        \param string the text to be displayed.
        \param font the font to be used.
        \param bgTexture the texture of the background sprite.
        \param style whether you want your lines to align to the center, left, or right
        \param dX how much wider the background is compared to the label
        \param dY how much taller the background is compared to the label

        \remarks should only be for initialization or change
    */
    void render(const char* string,
                VirtualResource font,
                VirtualResource bgTexture,
                LineStyle style = kLineCenter,
                float marginX = 0.5,
                float marginY = 0.5);


    /*!
        \brief sets the position along the x axis of the BorderedLabel.
        \param x the position to set it to.
    */
    void setxPosition(float x){
        if(line_style == kLineLeft) {
            Label::position[0] = x - Label::scale[0]*Label::max_width/2 -
                                    (Label::bounds.size[0]/125)*Label::scale[0];
        } else if(line_style == kLineRight) {
            Label::position[0] = x + Label::scale[0]*Label::max_width/2 - 
                                    (Label::bounds.size[0]/125)*Label::scale[0];
        } else { //Center
            Label::position[0] = x - (Label::bounds.size[0]/125)*Label::scale[0];
        }
        Sprite::position[0] = x;
    }

    /*!
        \brief sets the position along the y axis of the BorderedLabel.
        \param y the position to set it to.
    */
    void setyPosition(float y){
        if(!Label::isMultiLine) {
            Label::position[1] = y - Label::bounds.size[1] * 
                                Label::scale[1]/(2+(1/(Label::scale[1]*10)));
        } else {
            Label::position[1] = y + Label::bounds.size[1] *
                                        Label::scale[1]/(4.8+(1/(Label::scale[1]*10)));
        }
        
        Sprite::position[1] = y;
    }

    /*!
        \brief sets the position along the z axis of the BorderedLabel.
        \param z the position to set it to.
    */
    void setzPosition(float z){
        Label::position[2] = z;
        Sprite::position[2] = z;
    }

    /*!
        \brief sets the x, y, and z positions of the BorderedLabel
        \param x the position along the x axis
        \param y the position along the y axis
        \param z the position along the z axis
    */
    void setPosition(float x, float y, float z){
        setxPosition(x);
        setyPosition(y);
        setzPosition(z);
    }

    /*!
        \brief sets the scale along the x axis of the BorderedLabel.
        \param x the scale to set it to.
    */
    void setxScale(float x){
        Label::scale[0] = x;
        Sprite::scale[0] = Label::bounds.size[0]*Label::scale[0] + marginX*Label::scale[0];
        setxPosition(Sprite::position[0]);
    }

    /*!
        \brief sets the scale along the y axis of the BorderedLabel.
        \param y the scale to set it to.
    */   
    void setyScale(float y){
        Label::scale[1] = y;
        Sprite::scale[1] = Label::bounds.size[1]*Label::scale[1] + marginY*Label::scale[1];
        setyPosition(Sprite::position[1]);
    }

    /*!
        \brief sets the scale along the z axis of the BorderedLabel.
        \param z the scale to set it to.
    */
    void setzScale(float z){
        Label::scale[2] = z;
        Sprite::scale[2] = Label::bounds.size[2]*Label::scale[2];
    }

    /*!
        \brief sets the dimensions along the x axis of the BorderedLabel.
        \param x the scale to set it to.
    */
    void setxDimensions(float x){
        Label::dimensions[0] = x;
        Sprite::dimensions[0] = x;
    }

    /*!
        \brief sets the dimensions along the y axis of the BorderedLabel.
        \param y the scale to set it to.
    */
    void setyDimensions(float y){
        Label::dimensions[1] = y;
        Sprite::dimensions[1] = y;
    }

    /*!
        \brief sets the dimensions along the z axis of the BorderedLabel.
        \param z the scale to set it to.
    */
    void setzDimensions(float z){
        Label::dimensions[2] = z;
        Sprite::dimensions[2] = z;
    }

    /*!
        \brief a getter function for the position of the BorderedLabel along the x axis
        \param sprite whether to return the position of the sprite, or the label
        \return the position along the x axis
    */
    float xPosition(bool sprite = true){
        return sprite ? Sprite::position[0] : Label::position[0];
    }

    /*!
        \brief a getter function for the position of the BorderedLabel along the y axis
        \param sprite whether to return the position of the sprite, or the label
        \return the position along the y axis
    */
    float yPosition(bool sprite = true){
        return sprite ? Sprite::position[1] : Label::position[1];
    }

    /*!
        \brief a getter function for the position of the BorderedLabel along the z axis
        \param sprite whether to return the position of the sprite, or the label
        \return the position along the z axis
    */
    float zPosition(bool sprite = true){
        return sprite ? Sprite::position[2] : Label::position[2];
    }

    /*!
        \brief a getter function for the scale of the BorderedLabel along the x axis
        \param sprite whether to return the scale of the sprite, or the label
        \return the scale along the x axis
    */
    float xScale(bool sprite = true){
        return sprite ? Sprite::scale[0] : Label::scale[0];
    }

    /*!
        \brief a getter function for the scale of the BorderedLabel along the y axis
        \param sprite whether to return the scale of the sprite, or the label
        \return the scale along the y axis
    */
    float yScale(bool sprite = true){
        return sprite ? Sprite::scale[1] : Label::scale[1];
    }

    /*!
        \brief a getter function for the scale of the BorderedLabel along the z axis
        \param sprite whether to return the scale of the sprite, or the label
        \return the scale along the z axis
    */
    float zScale(bool sprite = true){
        return sprite ? Sprite::scale[2] : Label::scale[2];
    }

    /*!
        \brief a getter function for the dimensions of the BorderedLabel along the x axis
        \param sprite whether to return the dimensions of the sprite, or the label
        \return the dimensions along the x axis
    */
    float xDimensions(bool sprite = true){
        return sprite ? Sprite::dimensions[0] : Label::dimensions[0];
    }

    /*!
        \brief a getter function for the dimensions of the BorderedLabel along the y axis
        \param sprite whether to return the dimensions of the sprite, or the label
        \return the dimensions along the y axis
    */
    float yDimensions(bool sprite = true){
        return sprite ? Sprite::dimensions[1] : Label::dimensions[1];
    }

    /*!
        \brief a getter function for the dimensions of the BorderedLabel along the z axis
        \param sprite whether to return the dimensions of the sprite, or the label
        \return the dimensions along the z axis
    */
    float zDimensions(bool sprite = true){
        return sprite ? Sprite::dimensions[2] : Label::dimensions[2];
    }

    /*!
        \brief gets the redness of the BorderedLabel
        \param sprite whether to return the redness of the sprite, or the label
        \return the redness of the sprite or label
    */
    float red(bool sprite = false){
        return sprite ? Sprite::color[0] : Label::color[0];
    }

    /*!
        \brief gets the greenness of the BorderedLabel
        \param sprite whether to return the greenness of the sprite, or the label
        \return the greenness of the sprite or label
    */
    float green(bool sprite = false){
        return sprite ? Sprite::color[1] : Label::color[1];
    }

    /*!
        \brief gets the blueness of the BorderedLabel
        \param sprite whether to return the blueness of the sprite, or the label
        \return the blueness of the sprite or label
    */
    float blue(bool sprite = false){
        return sprite ? Sprite::color[2] : Label::color[2];
    }

    /*!
        \brief gets the opacity of the BorderedLabel
        \param sprite whether to return the opacity of the sprite, or the label
        \return the opacity of the sprite or label
    */
    float alpha(bool sprite = false){
        return sprite ? Sprite::color[3] : Label::color[3];
    }
    
private:
    const float BGOFFSET = 1/100;
    float marginX;
    float marginY;

};
}
}

#endif
