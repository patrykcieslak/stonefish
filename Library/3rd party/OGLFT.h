// -*- c++ -*-
/*
 * OGLFT: A library for drawing text with OpenGL using the FreeType library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: OGLFT.h,v 1.15 2003/10/01 14:41:09 allen Exp $
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef OGLFT_H
#define OGLFT_H

#include "OpenGLPipeline.h"

#include <cmath>
#include <map>
#include <list>
#include <vector>
#ifdef HAVE_MPATROL
#include <mpdebug.h>
#endif

#ifndef OGLFT_NO_SOLID
#include <GL/gle.h>
#endif

#ifndef OGLFT_NO_QT
#include <qstring.h>
#include <qcolor.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

//! All of OGLFT C++ objects are in this namespace.

namespace OGLFT {

  //! Thanks to DesCartes, I'd consider these manifest constants.
  enum Coordinates {
    X, //!< The X component of space
    Y, //!< The Y component of space
    Z, //!< The Z component of space
    W  //!< The projection component of space
  };

  //! Who to credit? Newton? I'd consider these manifest constants.
  enum ColorSpace {
    R, //!< The Red component of a color
    G, //!< The Green component of a color
    B, //!< The Blue component of a color
    A, //!< The Alpha (or transparency) of a color
  };

  //! Callback from GLU tessellation routines.
  typedef void (*GLUTessCallback)();

  //! The FreeType library instance.
  /*!
   * The FreeType library has a single, global instance of a library
   * handle.  This reference is used to load font faces. This detail
   * is generally hidden from the user of OGLFT, however, it
   * can be useful to get the FT_Library instance if you want to open
   * a font file yourself, either from disk or embedded in the program.
   */
  class Library {
  public:
    /*!
     * The FreeType library's library handle is only available through this
     * accessor method.
     * \return the global OGLFT FreeType library handle.
     */
    static FT_Library& instance ( void );

  protected:
    /*!
     * The constructor for this class is automatically called when
     * this library is loaded. Access the instance through the instance()
     * method.
     */
    Library ( void );
    /*!
     * This destructor is automatically called when the program exits.
     */
    ~Library( void );

  private:
    static Library library;
    static FT_Library library_;
  };

  //! Advance describes the "advance" of a glyph, namely the distance in
  //! model space at which the NEXT glyph should be drawn. This class exists
  //! to assist the computation of string metrics.
  struct Advance {
    float dx_;  //!< Advance increment in the X direction.
    float dy_;  //!< Advance increment in the Y direction.

    //! Default constructor. An otherwise uninitialized Advance contains zeros.
    Advance ( float dx = 0, float dy = 0 ) : dx_( dx ), dy_( dy )
    {}

    //! Initialize an advance from a FreeType advance member.
    Advance ( FT_Vector v )
    {
      dx_ = v.x / 64.;
      dy_ = v.y / 64.;
    }

    //! Increment Advance with a FreeType advance member.
    //! \return a reference to oneself.
    Advance& operator+= ( const FT_Vector v )
    {
      dx_ += v.x / 64.;
      dy_ += v.y / 64.;
      return *this;
    }
  };

  //! Describe the metrics of a glyph or string relative to the origin
  //! of the first character
  struct BBox {
    float x_min_;     //!< The left-most position at which "ink" appears.
    float y_min_;     //!< the bottom-most position at which "ink" appears.
    float x_max_;     //!< The right-most position at which "ink" appears.
    float y_max_;     //!< The top-most position at which "ink" appears.
    Advance advance_; //!< The (total) advancement

    //! Default constructor is all zeros.
    BBox () : x_min_( 0 ), y_min_( 0 ), x_max_( 0 ), y_max_( 0 )
    {}

    /*!
     *(Partially) initialize a BBox from a FreeType bounding box member.
     *(The advancement is initialized to zero by its default constructor).
     * \param ft_bbox a FreeType bounding box as retrieved from
     * \c FT_Glyph_Get_CBox.
     */
    BBox ( FT_BBox ft_bbox )
    {
      x_min_ = ft_bbox.xMin / 64.;
      y_min_ = ft_bbox.yMin / 64.;
      x_max_ = ft_bbox.xMax / 64.;
      y_max_ = ft_bbox.yMax / 64.;
    }

    //! Scale the bounding box by a constant.
    //! \param k a constant to scale the bounding box by.
    //! \return a reference to oneself.
    BBox& operator*= ( double k )
    {
      x_min_ *= k;
      y_min_ *= k;
      x_max_ *= k;
      y_max_ *= k;
      advance_.dx_ *= k;
      advance_.dy_ *= k;

      return *this;
    }

    /*!
     * Merge a bounding box into the current one (not really addition).
     * Each time a BBox is "added", the current BBox is expanded to include
     * the metrics of the new BBox. May only work for horizontal fonts, though.
     * \param b the bounding box to merge.
     * \return a reference to oneself.
     */
    BBox& operator+= ( const BBox& b )
    {
      float new_value;

      new_value = b.x_min_ + advance_.dx_;
      if ( new_value < x_min_ ) x_min_ = new_value;

      new_value = b.y_min_ + advance_.dy_;
      if ( new_value < y_min_ ) y_min_ = new_value;

      new_value = b.x_max_ + advance_.dx_;
      if ( new_value > x_max_ ) x_max_ = new_value;

      new_value = b.y_max_ + advance_.dy_;
      if ( new_value > y_max_ ) y_max_ = new_value;

      advance_.dx_ += b.advance_.dx_;
      advance_.dy_ += b.advance_.dy_;

      return *this;
    }
  };

  //! During tesselation of a polygonal Face (outline, filled or solid),
  //! an object which implements this interface can be used to compute a
  //! different color for each vertex.
  class ColorTess {
  public:
    //! Compute a color for this position. Note that the position is
    //! in the glyph's local coordinate system.
    //! \param p vertex position in glyph's local coordinate system. Argument is
    //! a GLdouble[3].
    //! \return GLfloat[4] (RGBA) color specification.
    virtual GLfloat* color ( GLdouble* p ) = 0;
  };

  //! During tesselation of a polygonal Face (outline, filled or solid),
  //! an object which implements this interface can be used to compute a
  //! different texture coordinate for each vertex.
  class TextureTess {
  public:
    //! Compute a texture coordinate for this position. Note that the
    //! position is in the glyph's local coordinate system.
    //! \param p vertex position in glyph's local coordinate system. Argument is
    //! a GLdouble[3].
    //! \return GLfloat[2] (s,t) texture coordinates.
    virtual GLfloat* texCoord ( GLdouble* p ) = 0;
  };

  //! The argument to setCharacterDisplayLists is an STL vector of
  //! OpenGL display list names (GLuints).
  typedef std::vector<GLuint> DisplayLists;

  //! A convenience definition of an iterator for display list vectors.
  typedef DisplayLists::const_iterator DLCI;

  //! A convenience definition of an iterator for display list vectors.
  typedef DisplayLists::iterator DLI;

  //! A face (aka font) used to render text with OpenGL.
  /*!
   * This is an abstract class, but it does define most the functions that
   * you are likely to call to manipulate the rendering of the text.
   */
  class Face {
  public:
    //! Thanks to the standard formerly known as PHIGS. Horizontal text
    //! justification constants.
    enum HorizontalJustification {
      LEFT,   //!< Left justified justification of text
      ORIGIN, //!< Natural origin alignment of text (default)
      CENTER, //!< Center justified alignment of text
      RIGHT   //!< Right justified alignment of text
    };

    //! Thanks to the standard formerly known as PHIGS. Vertical text
    //! justification constants.
    enum VerticalJustification {
      BOTTOM,   //!< Descender alignment of text
      BASELINE, //!< Baseline alignment of text (default)
      MIDDLE,   //!< Centered alignment of text
      TOP       //!< Ascender justification of text
    };

    //! Control how OpenGL display lists are created for individual glyphs.
    //! The default mode is to create display lists for each glyph as it
    //! is requested. Therefore, the Face drawing routines cannot themselves
    //! be called from within an open display list. In IMMEDIATE mode,
    //! cached glyphs will be drawn if available, otherwise the FreeType
    //! data for a glyph is re-rendered each time.
    enum GlyphCompileMode {
      COMPILE,    //!< Compile new glyphs when seen for the first time.
      IMMEDIATE   //!< Do not \em create display lists for glyphs.
    };

  private:
    //! We allow a Face to be constructed either from a file name
    //! or passed in as an already opened FreeType FT_Face. In the case
    //! of the later (already opened), we don't close the FT_Face on
    //! destruction. This way you can share FT_Faces between related
    //! OGLFT faces. Also, we're experimenting with being able to use
    //! multiple FT_Faces in a single OGLFT Face, so this is represented
    //! as a data structure.
    struct FaceData {
      FT_Face face_;
      bool free_on_exit_;
      FaceData ( FT_Face face, bool free_on_exit = true )
	: face_( face ), free_on_exit_( free_on_exit )
      {}
    };
  protected:
    //! The FreeType face - experimentally, this is now an array of
    //! faces so that we can handle a wider range of UNICODE points
    //! in case a face doesn't cover the points of interest.
    std::vector< FaceData > faces_;

    //! Did a font load OK?
    bool valid_;

    //! Glyph display list creation mode.
    enum GlyphCompileMode compile_mode_;

    //! Nominal point size.
    float point_size_;

    //! Display resolution in pixels per inch.
    FT_UInt resolution_;

    //! Does rendering text affect the MODELVIEW matrix?
    bool advance_;

    //! Foreground color (I really wanted to avoid this, but not really
    //! possible without state queries, which you can't put into
    //! display lists. Anyway, you'll be able to get even more fancy
    //! by passing in a function to map the color with, so why balk at
    //! this?)
    GLfloat foreground_color_[4];

    //! Background color (what modes would use this?)
    GLfloat background_color_[4];

    //! PHIGS-like horizontal positioning of text.
    enum HorizontalJustification horizontal_justification_;
    
    //! PHIGS-like vertical positioning of text.
    enum VerticalJustification vertical_justification_;

    //! Rotate an entire string in the Z plane
    GLfloat string_rotation_;

    //! Let the user decide which character to use as the rotation reference.
    //! Use "o" by default, I suppose.
    FT_UInt rotation_reference_glyph_;

    //! The rotation reference character could be in any face.
    FT_Face rotation_reference_face_;

    //! These are the translation offsets provided by the rotation reference
    //! character; for whom, we've discovered, only the Y position is relevant.
    GLfloat rotation_offset_y_;

    //! Type of the cache of defined glyph to display list mapping.
    typedef std::map< FT_UInt, GLuint > GlyphDLists;

    //! A convenience definition of the iterator over the glyph to display
    //! list map.
    typedef GlyphDLists::const_iterator GDLCI;

    //! A convenience definition of the iterator over the glyph to display
    //! list map.
    typedef GlyphDLists::iterator GDLI;

    //! Cache of defined glyph display lists
    GlyphDLists glyph_dlists_;

    //! The user can supply an array of display list which are invoked
    //! before each glyph is rendered.
    DisplayLists character_display_lists_;

  public:
    /*!
     * Construct a Face by loading a font from the given file.
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Face ( const char* filename, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * Alternatively, the user may have already opened a face and just
     * wants to draw with it. This is useful for Multiple Master fonts or
     * combining multiple files to increase UNICODE point coverage.
     * \param face open Freetype FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Face ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * Deleting a Face frees its FreeType face (and anything else it's
     * styles have allocated).
     */
    virtual ~Face ( void );

    /*!
     * Let the user test to see if the font was loaded OK.
     * \return true if the FT_Face was successfully created.
     */
    bool isValid ( void ) const { return valid_; }

    /*!
     * Add another FT_Face to the OGLFT Face. Generally used to add more
     * coverage of UNICODE points (at least that's the plan). This
     * routine takes a filename and takes ownership of the FT_Face.
     * \param filename name of file containing font face data.
     * \return true if face was successfully added.
     */
    bool addAuxiliaryFace ( const char* filename );

    /*!
     * Add another FT_Face to the OGLFT Face. Generally used to add more
     * coverage of UNICODE points (at least that's the plan). This
     * routine takes an already open FT_Face. The user is responsible
     * for clean up.
     * \param face open FreeType FT_Face
     * \return true if face was successfully added.
     */
    bool addAuxiliaryFace ( FT_Face face );

    /*!
     * By default, each time a new character is seen, its glyph is rendered
     * into a display list. This means that a display list cannot already
     * be open (since OpenGL doesn't allow nested display list creation).
     * Rendering can be set into immediate mode in which case glyphs are
     * rendered from display lists if available, but are otherwise generated
     * anew each time.
     * \param compile_mode the new compile mode.
     */
    void setCompileMode ( enum GlyphCompileMode compile_mode )
    {
      compile_mode_ = compile_mode;
    }

    /*!
     * \return the current glyph compile mode.
     */
    enum GlyphCompileMode compileMode ( void ) const { return compile_mode_; }

    /*!
     * For the rasterized styles (Monochrome, Grayscale, Translucent, Texture),
     * glyphs are rendered at the pixel size given by:
     *
     * point_size [pts] * / 72 [pts/in] * resolution [dots/in] = [dots].
     *
     * For the polygon styles (Outline, Filled, Solid), the "nominal" size of
     * the glyphs is:
     *
     * point_size[pts] / 72 [pts/in] * resolution [dots/in]
     * / units_per_EM [font unit/EM] = [dots * EM].
     *
     * If the MODELVIEW and PROJECTION matrices are such that one screen pixel
     * corresponds to one modeling unit, then polygonal Faces will
     * be the same size as raster Faces.
     *
     * Note that changing the point size after Face creation will invalidate
     * the cache of OpenGL display lists and any other information which
     * the individual styles have cached.
     * \param point_size the new point size in points (1/72-th inch).
     */
    void setPointSize ( float point_size );
      
    /*!
     * \return the current point size.
     */
    float pointSize ( void ) { return point_size_; }

    /*!
     * For the rasterized styles (Monochrome, Grayscale,
     * Translucent, Texture), the exact rendered size of the glyphs depends on
     * the resolution of the display (as opposed to the polygon styles
     * whose size is controlled by the viewing matrices). The Texture
     * style is slightly different because the glyphs are texture-mapped
     * onto an arbitrary rectangle; here, the resolution only controls
     * how accurately the glyph is rendered.
     * \param resolution the resolution in DPI (dots per inch).
     */
    void setResolution ( FT_UInt resolution );
      
    /*!
     * \return the current raster resolution.
     */
    FT_UInt resolution ( void ) { return resolution_; }

    /*!
     * If advance is true, then the changes made to the MODELVIEW matrix
     * to render a string are allowed to remain. Otherwise, the library
     * pushes the current MODELVIEW matrix onto the matrix stack, renders
     * the string and then pops it off again. Rendering a character always
     * modifies the MODELVIEW matrix.
     * \param advance whether or not the MODELVIEW matrix should be left
     * translated by the advancement of a rendered string.
     */
    void setAdvance ( bool advance ) { advance_ = advance; }

    /*!
     * \return the advance value.
     */
    bool advance ( void ) const { return advance_; }

    /*!
     * This is the nominal color of the glyphs. A lot of other things
     * can alter what you actually see! Note that changing the foreground
     * color invalidates the glyph cache.
     * \param red the red component of the foreground color.
     * \param green the green component of the foreground color.
     * \param blue the blue component of the foreground color.
     * \param alpha the alpha component of the foreground color.
     */
    void setForegroundColor ( GLfloat red = 0.0,
			      GLfloat green = 0.0,
			      GLfloat blue = 0.0,
			      GLfloat alpha = 1.0 );

    /*!
     * This is the nominal color of the glyphs. A lot of other things
     * can alter what you actually see! Note that changing the foreground
     * color invalidates the glyph cache.
     * \param foreground_color an array of 4 values corresponding to the
     * red, green, blue and alpha components of the foreground color.
     */
    void setForegroundColor ( const GLfloat foreground_color[4] );
#ifndef OGLFT_NO_QT
    /*!
     * This is the nominal color of the glyphs. A lot of other things
     * can alter what you actually see! Note that changing the foreground
     * color invalidates the glyph cache.
     * \param foreground_color the foreground color as an unsigned int.
     */
    void setForegroundColor ( const QRgb foreground_color );
#endif /* OGLFT_NO_QT */
    /*!
     * \return the red component of the foreground color
     */
    GLfloat foregroundRed ( void ) const { return foreground_color_[R]; }
    /*!
     * \return the green component of the foreground color
     */
    GLfloat foregroundGreen ( void ) const { return foreground_color_[G]; }
    /*!
     * \return the blue component of the foreground color
     */
    GLfloat foregroundBlue ( void ) const { return foreground_color_[B]; }
    /*!
     * \return the alpha component of the foreground color
     */
    GLfloat foregroundAlpha ( void ) const { return foreground_color_[A]; }

    /*!
     * This is the nominal background color of the glyphs. A lot of other things
     * can alter what you actually see! Note that changing the background
     * color invalidates the glyph cache.
     * \param red the red component of the background color.
     * \param green the green component of the background color.
     * \param blue the blue component of the background color.
     * \param alpha the alpha component of the background color.
     */
    void setBackgroundColor ( GLfloat red = 1.0,
			      GLfloat green = 1.0,
			      GLfloat blue = 1.0,
			      GLfloat alpha = 0.0 );

    /*!
     * This is the nominal background color of the glyphs. A lot of other things
     * can alter what you actually see! Note that changing the background
     * color invalidates the glyph cache.
     * \param background_color an array of 4 values corresponding to the
     * red, green, blue and alpha components of the background color.
     */
    void setBackgroundColor ( const GLfloat background_color[4] );
#ifndef OGLFT_NO_QT
    /*!
     * This is the nominal background color of the glyphs. A lot of other things
     * can alter what you actually see! Note that changing the background
     * color invalidates the glyph cache.
     * \param background_color the background color as an unsigned int.
     */
    void setBackgroundColor ( const QRgb background_color );
#endif /* OGLFT_NO_QT */
    /*!
     * \return the red component of the background color
     */
    GLfloat backgroundRed ( void ) const { return background_color_[R]; }
    /*!
     * \return the green component of the background color
     */
    GLfloat backgroundGreen ( void ) const { return background_color_[G]; }
    /*!
     * \return the blue component of the background color
     */
    GLfloat backgroundBlue ( void ) const { return background_color_[B]; }
    /*!
     * \return the alpha component of the background color
     */
    GLfloat backgroundAlpha ( void ) const { return background_color_[A]; }

    /*!
     * Set the individual character rotation in the Z direction.
     * \param character_rotation_z angle in degrees of z rotation.
     */
    virtual void setCharacterRotationZ ( GLfloat character_rotation_z ) = 0;

    /*!
     * \return the character rotation in the Z direction.
     */
    virtual GLfloat characterRotationZ ( void ) const = 0;

    /*!
     * The z rotation angle needs a center. Nominate a character whose
     * center is to be the center of rotation. By default, use "o".
     * \param c rotation reference character.
     */
    void setCharacterRotationReference ( unsigned char c );

    /*!
     * Rotate an entire string through the given angle (in the Z plane only).
     * (Somewhat pointless for the vector styles since you can do mostly
     * the same thing with the MODELVIEW transform, however, for what its
     * worth, this routine uses the FreeType rotation function to compute
     * the "proper" metrics for glyph advance.)
     * \param string_rotation angle in degrees of z rotation.
     */
    void setStringRotation ( GLfloat string_rotation );

    /*!
     * \return the (Z plane) string rotation angle.
     */
    GLfloat stringRotation ( void ) const { return string_rotation_; }

    /*!
     * Set the horizontal justification.
     * \param horizontal_justification the new horizontal justification.
     */
    void setHorizontalJustification ( enum HorizontalJustification
				      horizontal_justification )
    {
      horizontal_justification_ = horizontal_justification;
    }

    /*!
     * \return the horizontal justification.
     */
    enum HorizontalJustification horizontalJustification ( void ) const
    { return horizontal_justification_; }

    /*!
     * Set the vertical justification.
     * \param vertical_justification the new vertical justification
     */
    void setVerticalJustification ( enum VerticalJustification
				    vertical_justification )
    {
      vertical_justification_ = vertical_justification;
    }

    /*!
     * \return the vertical justification.
     */
    enum VerticalJustification verticaljustification ( void )
      const { return vertical_justification_; }

    /*!
     * Specify an OpenGL display list to be invoked before
     * each character in a string. Face makes a copy of the argument. Pass
     * an empty DisplayLists to disable this feature.
     * \param character_display_lists STL vector<GLuint> containing a display
     * list to invoke before each glyph in a string is drawn.
     */
    void setCharacterDisplayLists ( const DisplayLists& character_display_lists )
    {
      character_display_lists_ = character_display_lists;
    }

    /*!
     * \return a reference to the array of character display lists. This is
     * the live list as stored in the Face.
     */
    DisplayLists& characterDisplayLists ( void )
    { return character_display_lists_; }

    /*!
     * \return the height (i.e., line spacing) at the current character size.
     */
    virtual double height ( void ) const = 0;

    /*!
     * Compute the bounding box info for a character.
     * \param c the (latin1) character to measure.
     * \return the bounding box of c.
     */
    virtual BBox measure ( unsigned char c ) = 0;
#ifndef OGLFT_NO_QT
    /*!
     * Compute the bounding box info for a character.
     * \param c the (UNICODE) character to measure.
     * \return the bounding box of c.
     */
    virtual BBox measure ( QChar c ) = 0;
#endif /* OGLFT_NO_QT */
    /*!
     * Compute the bounding box info for a string.
     * \param s the (latin1) string to measure.
     * \return the bounding box of s.
     */
    virtual BBox measure ( const char* s );
    /*!
     * Compute the bounding box info for a string without conversion
     * to modeling coordinates.
     * \param s the (latin1) string to measure.
     * \return the bounding box of s.
     */
    virtual BBox measureRaw ( const char* s );
#ifndef OGLFT_NO_QT
    /*!
     * Compute the bounding box info for a string.
     * \param s the (UNICODE) string to measure.
     * \return the bounding box of s.
     */
    virtual BBox measure ( const QString& s );
    /*!
     * Compute the bounding box info for a real number formatted as specified.
     * \param format (see draw for valid formats)
     * \param number real number.
     * \return the bounding box of the formatted number.
     */
    virtual BBox measure ( const QString& format, double number );
    /*!
     * Compute the bounding box info for a string without conversion
     * to modeling coordinates.
     * \param s the (UNICODE) string to measure.
     * \return the bounding box of s.
     */
    virtual BBox measureRaw ( const QString& s );
#endif /* OGLFT_NO_QT */
    /*!
     * Compile a string into an OpenGL display list for later
     * rendering.  Essentially, the string is rendered at the origin
     * of the current MODELVIEW. Note: no other display lists should
     * be open when this routine is called. Also, the Face does not
     * keep track of these lists, so you must delete them in order
     * to recover the memory.
     * \param s the (latin1) string to compile.
     * \return the display list name for the string.
     */
    GLuint compile ( const char* s );
#ifndef OGLFT_NO_QT
    /*!
     * Compile a string into an OpenGL display list for later
     * rendering.  Essentially, the string is rendered at the origin
     * of the current MODELVIEW. Note: no other display lists should
     * be open when this routine is called. Also, the Face does not
     * keep track of these lists, so you must delete them in order
     * to recover the memory.
     * \param s the (UNICODE) string to compile.
     * \return the display list name for the string.
     */
    GLuint compile ( const QString& s );
#endif /* OGLFT_NO_QT */
    /*!
     * Compile a single character (glyph) into an OpenGL display list
     * for later rendering. The Face \em does keep track of these
     * display lists, so do not delete them.
     * \param c the (latin1) character to compile.
     * \return the display list name for the character.
     */
    GLuint compile ( unsigned char c );
#ifndef OGLFT_NO_QT
    /*!
     * Compile a single character (glyph) into an OpenGL display list
     * for later rendering. The Face \em does keep track of these
     * display lists, so do not delete them.
     * \param c the (UNICODE) character to compile.
     * \return the display list name for the character.
     */
    GLuint compile ( const QChar c );
#endif /* OGLFT_NO_QT */
    /*!
     * Draw a (latin1) string using the current MODELVIEW matrix. If
     * advance is true, then the final glyph advance changes to the
     * MODELVIEW matrix are left in place.
     * \param s the (latin1) string to draw.
     */
    void draw ( const char* s );
#ifndef OGLFT_NO_QT
    /*!
     * Draw a (UNICODE) string using the current MODELVIEW
     * matrix. If advance is true, then the final glyph advance
     * changes to the MODELVIEW matrix are left in place.
     * \param s the (UNICODE) string to draw.
     */
    void draw ( const QString& s );
#endif /* OGLFT_NO_QT */
    /*!
     * Draw the character using the current MODELVIEW matrix. Note that
     * the MODELVIEW matrix is modified by the glyph advance. Draw a
     * string if you don't want the MODELVIEW matrix changed.
     * \param c the (latin1) character to draw.
     */
    void draw ( unsigned char c );

#ifndef OGLFT_NO_QT
    /*!
     * Draw the character using the current MODELVIEW matrix. Note that
     * the MODELVIEW matrix is modified by the glyph advance. Draw a
     * string if you don't want the MODELVIEW matrix changed.
     * \param c the (UNICODE) character to draw.
     */
    void draw ( const QChar c );
#endif /* OGLFT_NO_QT */
    /*!
     * Draw the (latin1) character at the given 2D point. Note that
     * the MODELVIEW matrix is modified by the glyph advance. Draw
     * a string if you don't want the MODELVIEW matrix changed.
     * \param x the X position.
     * \param y the Y position.
     * \param c the (latin1) character to draw.
     */
    void draw ( GLfloat x, GLfloat y, unsigned char c );
    /*!
     * Draw the (latin1) character at the given 3D point. Note that
     * the MODELVIEW matrix is modified by the glyph advance. Draw
     * a string if you don't want the MODELVIEW matrix changed.
     * \param x the X position.
     * \param y the Y position.
     * \param z the Z position.
     * \param c the (latin1) character to draw.
     */
    void draw ( GLfloat x, GLfloat y, GLfloat z, unsigned char c );
#ifndef OGLFT_NO_QT
    /*!
     * Draw the (UNICODE) character at the given 2D point. Note that
     * the MODELVIEW matrix is modified by the glyph advance. Draw
     * a string if you don't want the MODELVIEW matrix changed.
     * \param x the X position.
     * \param y the Y position.
     * \param c the (UNICODE) character to draw.
     */
    void draw ( GLfloat x, GLfloat y, QChar c );
    /*!
     * Draw the (UNICODE) character at the given 3D point. Note that
     * the MODELVIEW matrix is modified by the glyph advance. Draw
     * a string if you don't want the MODELVIEW matrix changed.
     * \param x the X position.
     * \param y the Y position.
     * \param z the Z position.
     * \param c the (UNICODE) character to draw.
     */
    void draw ( GLfloat x, GLfloat y, GLfloat z, QChar c );
#endif /* OGLFT_NO_QT */
    /*!
     * Draw a string at the given 2D point.
     * \param x the X position.
     * \param y the Y position.
     * \param s the (latin1) string to draw.
     */
    void draw ( GLfloat x, GLfloat y, const char* s );
    /*!
     * Draw a string at the given 3D point.
     * \param x the X position.
     * \param y the Y position.
     * \param z the Z position.
     * \param s the (latin1) string to draw.
     */
    void draw ( GLfloat x, GLfloat y, GLfloat z, const char* s );
#ifndef OGLFT_NO_QT
    /*!
     * Draw a string at the given 2D point.
     * \param x the X position.
     * \param y the Y position.
     * \param s the (UNICODE) string to draw.
     */
    void draw ( GLfloat x, GLfloat y, const QString& s );
    /*!
     * Draw a string at the given 3D point.
     * \param x the X position.
     * \param y the Y position.
     * \param z the Z position.
     * \param s the (UNICODE) string to draw.
     */
    void draw ( GLfloat x, GLfloat y, GLfloat z, const QString& s );
    /*!
     * Draw a real number per the given format at the given 2D point.
     * \param x the X position.
     * \param y the Y position.
     * \param format Like a typical printf format. Regular text is printed
     * while a '%' introduces the real number's format. Includes the
     * following format flags:
     * \li %%x.yf - floating point in field width x and precision y
     * \li %%x.ye - scientific notation in field width x and precision y
     * \li %%x.yg - pick best floating or scientific in field width x and
     *  precision y
     * \li %%p - draw as a proper fraction, e.g. 1 1/2. Note: this currently
     * requires a special font which encodes glyphs to be drawn for the
     * numerator and demoninator in the UNICODE Private Area (0xE000).
     *
     * \param number the numeric value.
     */
    void draw ( GLfloat x, GLfloat y, const QString& format, double number );
    /*!
     * Draw a real number per the given format at the given 3D point.
     * \param x the X position.
     * \param y the Y position.
     * \param z the Z position.
     * \param format Like a typical printf format. Regular text is printed
     * while a '%' introduces the real number's format. Includes the
     * following format flags:
     * \li %%x.yf - floating point in field width x and precision y
     * \li %%x.ye - scientific notation in field width x and precision y
     * \li %%x.yg - pick best floating or scientific in field width x and
     *  precision y
     * \li %%p - draw as a proper fraction, e.g. 1 1/2. Note: this currently
     * requires a special font which encodes glyphs to be drawn for the
     * numerator and demoninator in the UNICODE Private Area (0xE000).
     *
     * \param number the numeric value.
     */
    void draw ( GLfloat x, GLfloat y, GLfloat z, const QString& format,
		double number );
#endif /* OGLFT_NO_QT */
    /*!
     * \return the nominal ascender from the face. This is in "notional"
     * units.
     */
    int ascender ( void ) { return faces_.front().face_->ascender; }

    /*!
     * \return the nominal descender from the face. This is in "notional"
     * units.
     */
    int descender ( void ) { return faces_.front().face_->descender; }

  protected:
    // The various styles override these routines

    //! Some styles, in particular the Texture, need specialized steps
    //! to compile a glyph into an OpenGL display list.
    //! \param face the FT_Face containing the glyph.
    //! \param glyph_index the index of the glyph in face.
    //! \return the display list of the compiled glyph.
    virtual GLuint compileGlyph ( FT_Face face, FT_UInt glyph_index ) = 0;

    //! Each style implements its own glyph rendering routine.
    //! \param face the FT_Face containing the glyph.
    //! \param glyph_index the index of the glyph in face.
    virtual void renderGlyph ( FT_Face face, FT_UInt glyph_index ) = 0;

    //! There is a slight different between the way in which the polygonal
    //! and raster styles select the character size for FreeType to generate.
    virtual void setCharSize ( void ) = 0;

    //! The different styles have different caching needs (well, really only
    //! the texture style currently has more than the display list cache).
    virtual void clearCaches ( void ) = 0;

    //! The polygonal and raster styles compute different values for the
    //! Z rotation offset. (It's in integer pixels for the raster styles and
    //! in floating point pixels for the polygonal styles.)
    virtual void setRotationOffset ( void ) = 0;

  private:
    void init ( void );
    BBox measure_nominal ( const char* s );
#ifndef OGLFT_NO_QT
    BBox measure_nominal ( const QString& s );
    QString format_number ( const QString& format, double number );
#endif /* OGLFT_NO_QT */
  };

  //! This is the base class of the polygonal styles: outline, filled and solid.
  /*!
   * In the polygonal styles, the detailed geometric outlines of the glyphs
   * are extracted from the font file and rendered as polygons.
   */
  class Polygonal : public Face {
  protected:
    //! Angle of rotation of characters relative to text orientation.
    struct {
      bool active_;
      GLfloat x_, y_, z_;
    } character_rotation_;

    //! The tessellation of curves is pretty crude; regardless of length,
    //! use the same number of increments (and as near as I can tell, this
    //! is more than sufficient unless the glyph takes up the whole screen).
    unsigned int tessellation_steps_;

    //! When curves are tessellated, we use the forward difference algorithm
    //! from Foley and van Dam for parametric curves (pg. 511 of 2nd Ed. in C).
    //! So, the step size, delta, is in the parametric variable which is always
    //! on the interval [0,1]. Therefore, delta = 1/tessellation_steps
    double delta_, delta2_, delta3_;

    //! For vector rendition modes, FreeType is allowed to generate the
    //! lines and arcs at the original face definition resolution. To
    //! get to the proper glyph size, the vertices are scaled before
    //! they're passed to the GLU tessellation routines.
    double vector_scale_;

    //! Callbacks for FreeType glyph decomposition into outlines
    FT_Outline_Funcs interface_;

    //! Default number of steps to break TrueType and Type1 arcs into.
    //! (Note: this looks good to me, anyway)
    static const unsigned int DEFAULT_TESSELLATION_STEPS = 4;

    /*!
     * VertexInfo is a private class which is used by the decomposition and
     * tessellation routines to store the vertices and other data of the glyph's
     * outline.  Because of the "impedance mismatch" between the crazy
     * 26.6 fixed point format of the FreeType library (well, don't
     * blame them; look at what they have to work with) and OpenGL's preference
     * for double precision, this simple vector has two constructors: one
     * for 26.6 format and one for direct floating point.
     *
     * VertexInfo also contains (optional) pointers to objects which
     * implement the ColorTess and TextureTess interfaces.
     */
    struct VertexInfo {
      double v_[3]; //!< Why is this double precision? Because the second
		    //!< argument to the routine gluTessVertex is a pointer
		    //!< to an array of doubles. Otherwise, we could use
		    //!< single precision everywhere.

      //! The user can provide a ColorTess object which computes a color
      //! for each tesselated vertex.
      ColorTess* color_tess_;

      //! The user can provide a TextureTess object which computes texture
      //! coordinates for each tesselated vertex.
      TextureTess* texture_tess_;

      //! Default constructor just initializes Vertex to zero.
      //! \param color_tess optional color tesselation object.
      //! \param texture_tess optional texture tesselation object.
      VertexInfo ( ColorTess* color_tess = 0, TextureTess* texture_tess = 0 )
	: color_tess_( color_tess ), texture_tess_( texture_tess )
      {
	v_[X] = v_[Y] = v_[Z] = 0.;
      }

      /*!
       * Construct a Vertex from a point in a FreeType contour.
       * \param ft_v a FreeType FT_Vector, normally passed into the
       * the decomposition callbacks.
       * \param color_tess optional color tesselation object.
       * \param texture_tess optional texture tesselation object.
       */
      VertexInfo ( FT_Vector* ft_v, ColorTess* color_tess = 0,
		   TextureTess* texture_tess = 0 )
	: color_tess_( color_tess ), texture_tess_( texture_tess )
      {
	v_[X] = (double)( ft_v->x / 64 ) + (double)( ft_v->x % 64 ) / 64.;
	v_[Y] = (double)( ft_v->y / 64 ) + (double)( ft_v->y % 64 ) / 64.;
	v_[Z] = 0.;
      }

      /*!
       * Construct a Vertex from a 2D point.
       * \param p 2D array of doubles.
       * \param color_tess optional color tesselation object.
       * \param texture_tess optional texture tesselation object.
       */
      VertexInfo ( double p[2], ColorTess* color_tess = 0,
		   TextureTess* texture_tess = 0 )
	: color_tess_( color_tess ), texture_tess_( texture_tess )
      {
	v_[X] = p[X];
	v_[Y] = p[Y];
	v_[Z] = 0.;
      }

      /*!
       * Construct a Vertex from a 2D point.
       * \param x the X coordinate.
       * \param y the Y coordinate.
       * \param color_tess optional color tesselation object.
       * \param texture_tess optional texture tesselation object.
       */
      VertexInfo ( double x, double y, ColorTess* color_tess = 0,
		   TextureTess* texture_tess = 0 )
	: color_tess_( color_tess ), texture_tess_( texture_tess )
      {
	v_[X] = x;
	v_[Y] = y;
	v_[Z] = 0.;
      }

      //! Treat the Vertex like a vector: Normalize its length in the
      //! usual way.
      void normalize ( void )
      {
	double length = sqrt( v_[X] * v_[X] + v_[Y] * v_[Y] + v_[Z] * v_[Z] );
	v_[X] /= length;
	v_[Y] /= length;
	v_[Z] /= length;
      }
    };

    /*!
     * Buffers the last control point as the outline of a glyph is
     * decomposed.
     */
    VertexInfo last_vertex_;

    //! Normally, we will consider a list of vertices.
    typedef std::list< VertexInfo* > VertexInfoList;

    //! A convenience definition of the iterator over the list of vertices.
    typedef VertexInfoList::const_iterator VILCI;

    //! A convenience definition of the iterator over the list of vertices.
    typedef VertexInfoList::iterator VILI;

    /*!
     * As curves are decomposed out of the glyph, their vertices are passed
     * along to the GLU tessellation functions. These vertices have to
     * hang around until gluTessContourEnd is called.
     */
    VertexInfoList vertices_;

    //! As GLU tessellation proceeds, new contours open with every call
    //! to moveTo.
    bool contour_open_;

    //! The user can provide a ColorTess object which computes a color
    //! for each tesselated vertex.
    ColorTess* color_tess_;

    //! The user can provide a TextureTess object which computes texture
    //! coordinates for each tesselated vertex.
    TextureTess* texture_tess_;

  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Polygonal ( const char* filename, float point_size = 12,
		FT_UInt resolution = 100 );

    /*!
     * \param face open Freetype FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Polygonal ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * The Polygonal destructor doesn't do anything in particular.
     */
    virtual ~Polygonal ( void );

    /*!
     * TrueType and Type1 files describe the boundaries of glyphs with
     * quadratic and cubic curves, respectively. Since OpenGL can only really
     * draw straight lines, these curves have to be tessellated. The
     * number of steps used is fixed for all glyphs in the face,
     * but can be changed through this method. Other notes: This value is
     * only applicable for Outline, Filled and Solid styles. Changing this value
     * invalidates any cached display lists for glyphs in this face.
     *
     * \param tessellation_steps the number of steps to tessellate each curved
     * segment of a glyph outline.
     */
    void setTessellationSteps ( unsigned int tessellation_steps );

    /*!
     * \return the number of steps used to tessellate curves in the
     * polygonal font types.
     */
    unsigned int tessellationSteps ( void ) const { return tessellation_steps_; }

    /*!
     * Set the individual character rotation in the X direction.
     * \param character_rotation_x angle in degrees of the X rotation.
     */
    void setCharacterRotationX ( GLfloat character_rotation_x );

    /*!
     * Set the individual character rotation in the Y direction.
     * \param character_rotation_y angle in degrees of the Y rotation.
     */
    void setCharacterRotationY ( GLfloat character_rotation_y );

    /*!
     * Set the individual character rotation in the Z direction.
     * \param character_rotation_z angle in degrees of the Z rotation.
     */
    void setCharacterRotationZ ( GLfloat character_rotation_z );

    /*!
     * \return the character rotation in the X direction.
     */
    GLfloat characterRotationX ( void ) const { return character_rotation_.x_; }

    /*!
     * \return the character rotation in the Y direction.
     */
    GLfloat characterRotationY ( void ) const { return character_rotation_.y_; }

    /*!
     * \return the character rotation in the Z direction.
     */
    GLfloat characterRotationZ ( void ) const { return character_rotation_.z_; }

    /*!
     * Set an optional color tesselation object. Each tesselated vertex
     * is passed to this object, which returns a color for that position
     * in space.
     * \param color_tess the color tesselation object.
     */
    void setColorTess ( ColorTess* color_tess );
    /*!
     * \return the color tesselation object.
     */
    ColorTess* colorTess ( void ) const { return color_tess_; }
    /*!
     * Set an optional texture coordinate tesselation object. Each
     * tessellated vertex is passed to this object, which returns
     * texture coordinates for that position in space.
     * \param texture_tess the texture coordinate tesselation object.
     */
    void setTextureTess ( TextureTess* texture_tess );
    /*!
     * \return the texture coordinate tesselation object.
     */
    TextureTess* textureTess ( void ) const { return texture_tess_; }

    /*!
     * \return the height (i.e., line spacing) at the current character size.
     */
    double height ( void ) const;

    /*!
     * Implement measuring a character in a polygonal face.
     * \param c the (latin1) character to measure
     * \return the bounding box of c.
     */
    BBox measure ( unsigned char c );
#ifndef OGLFT_NO_QT
    /*!
     * Implement measuring a character in a polygonal face.
     * \param c the (UNICODE) character to measure
     * \return the bounding box of c.
     */
    BBox measure ( const QChar c );
#endif /* OGLFT_NO_QT */
    /*!
     * Measure a string of characters. Note: currently, this merely
     * calls Face's measure routine.
     * \param s string of (latin1) characters to measure
     * \return the bounding box of s.
     */
    BBox measure ( const char* s ) { return Face::measure( s ); }
#ifndef OGLFT_NO_QT
    /*!
     * Implement measuring a formatted number
     * \param format the format string
     * \param number to value to format
     * \return the bounding box of the formatted number
     */
    BBox measure ( const QString& format, double number )
    { return Face::measure( format, number ); }
#endif /* OGLFT_NO_QT */

  private:
    void init ( void );
    void setCharSize ( void );
    void setRotationOffset ( void );
    GLuint compileGlyph ( FT_Face face, FT_UInt glyph_index );
  protected:
    void clearCaches ( void );
  };

  //! Render text as a polygon outline.
  /*!
   * \image html outline_class.png
   * Text is drawn as an outline of each glyph. The contours are extracted
   * from the font file through FreeType. FreeType is used to scale the
   * contours to a given size. Usually the outline is drawn in the foreground
   * color, however, you can specify a ColorTess object to provide a color
   * for each vertex individually. You can also use
   * the per-glyph display list functionality to alter the attributes
   * of each glyph.
   *
   * The only complexity to this style is selecting the point size. Since
   * the outlines are drawn as a polygon, they are subject to the MODELVIEW
   * transformation. The point size is nominally chosen to be the same as a
   * raster image generated at the given resolution. Some experimentation
   * with point size and resolution may be necessary to achieve the desired
   * results.
   */
  class Outline : public Polygonal {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Outline ( const char* filename, float point_size = 12,
	      FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Outline ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * The destructor doesn't do anything in particular.
     */
    ~Outline ( void );
  private:
    void init ( void );
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
    static int moveToCallback ( FT_Vector* to, Outline* outline );
    static int lineToCallback ( FT_Vector* to, Outline* outline );
    static int conicToCallback ( FT_Vector* control, FT_Vector* to, Outline* outline );
    static int cubicToCallback ( FT_Vector* control1, FT_Vector* control2,
				 FT_Vector* to, Outline* outline );
  };

  //! Render text as a filled polygons.
  /*!
   * \image html filled_class.png
   * Each glyph is drawn as a filled polygon. The contours are extracted
   * from the font file through FreeType. FreeType is used to scale the
   * contours to the given size. Then the GLU tessellation routines are used
   * to tessellate the contours into polygons (well, triangles). By default,
   * these are drawn in GL_FILL polygon mode, but any other polygon mode
   * can be specified.
   *
   * Usually, the polygons are drawn only in the
   * foreground color, however, you may supply ColorTess and TextureTess
   * objects which can alter the color or texture coordinates of each
   * vertex individually. You can also use
   * the per-glyph display list functionality to alter the attributes
   * of each glyph.
   *
   * The only complexity to this style is selecting the point size. Since
   * the glyphs are drawn as polygons, they are subject to the viewing and
   * modeling transformations. The point size is nominally chosen to be the same
   * as a raster image generated at the given resolution. Some experimentation
   * with point size and resolution may be necessary to achieve the desired
   * results.
   */
  class Filled : public Polygonal {
    //! 3D tessellation of glyphs is accomplished through the standard GLU
    //! routines
    GLUtesselator* tess_obj_;

    //! A place to store any extra vertices generated by the Combine callback
    VertexInfoList extra_vertices_;

  protected:
    //! Offset the glyph in the Z direction. Solely for the Solid subclass.
    //! Until I can figure out how to shift the glyph outside the context
    //! of this class, I guess this has got to stay (but it is redundant
    //! to extrusion_.depth_)
    GLfloat depth_offset_;

  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Filled ( const char* filename, float point_size = 12,
	     FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Filled ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
    /*!
     * The destructor deletes the GLU tessellation object allocated in
     * in the constructor.
     */
    virtual ~Filled ( void );

    /*!
     * \return the list of extra vertices created by the GLU tessellation
     * combine callback.
     */
    VertexInfoList& extraVertices ( void ) { return extra_vertices_; }

  protected:
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
  private:
    void init ( void );
    static int moveToCallback ( FT_Vector* to, Filled* filled );
    static int lineToCallback ( FT_Vector* to, Filled* filled );
    static int conicToCallback ( FT_Vector* control, FT_Vector* to, Filled* filled);
    static int cubicToCallback ( FT_Vector* control1, FT_Vector* control2,
				 FT_Vector* to, Filled* filled );
    static void vertexCallback ( VertexInfo* vertex );
    static void beginCallback ( GLenum which );
    static void endCallback ( void );
    static void combineCallback ( GLdouble coords[3], void* vertex_data[4],
				  GLfloat weight[4], void** out_data,
				  Filled* filled );
    static void errorCallback ( GLenum error_code );
  };

#ifndef OGLFT_NO_SOLID
  //! Render text as solid letters.
  /*!
   * \image html solid_class.png
   * Each glyph is drawn as a closed solid. The contours are extracted
   * from the font file through FreeType. FreeType is used to scale the
   * contours to the given size. The contours are passed to the GLE
   * tubing and extrusion library to create the sides of the solid.
   * Then the GLU tessellation routines are used
   * to tessellate the contours into polygons which are used to cap the sides.
   *
   * Currently, the solids are drawn only in the foreground color. However,
   * proper surface normals are computed so that the solids may be lighted.
   * Eventually, you'll be able to supply a color/texture
   * coordinate function to make glyphs more interesting. Note that you can use
   * the per-glyph display list functionality to alter each glyph individually.
   *
   * Another TODO item is to improve the interaction with GLE. Currently,
   * you can only create block solids. Eventually, we'll have the capability
   * add bevels and rounds to the edges of the solids and maybe even more
   * general extrusions (like, for example, the swooshing letters in the title
   * sequence of the Salkind's 1978 "Superman" movie).
   *
   * The only complexity to this style is selecting the point size. Since
   * the glyphs are drawn as a collection of polygons, they are subject to the
   * viewing and modeling transformations. The point size is nominally chosen
   * to be the same as a raster image generated at the given resolution.
   * Some experimentation with point size and resolution may be necessary to
   * achieve the desired results.
   */
  class Solid : public Filled {
  private:

    //! Callbacks for FreeType glyph decomposition into outlines (note: this
    //! has the same name as the variable in Polygonal, but it is distinct since
    //! the routines for the GLE contouring are different from the Filled
    //! GLU tessellation routines. This may be too confusing?)
    FT_Outline_Funcs interface_;

    //! For now, you can only get block extruded solids
    static const unsigned int N_POLYLINE_PTS = 4;

    //! Data for the gleExtrusion routine
    struct glePoint2D {
      double p_[2];
      glePoint2D ( double p[2] ) { p_[X] = p[X]; p_[Y] = p[Y]; }
      glePoint2D ( double x, double y ) { p_[X] = x; p_[Y] = y; }
      glePoint2D ( const VertexInfo& v ) { p_[X] = v.v_[X]; p_[Y] = v.v_[Y]; }
    };

    //! Collect all the output from GLE in one of these structures.
    struct {
      double depth_;
      struct {
	int x_, y_;
      } normal_sign_;
      std::vector< glePoint2D > contour_;
      std::vector< glePoint2D > contour_normals_;
      gleDouble up_[3];
      int n_polyline_pts_;
      gleDouble point_array_[N_POLYLINE_PTS][3];
    } extrusion_;

  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Solid ( const char* filename, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Solid ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * The destructor doesn't do anything in particular.
     */
    ~Solid ( void );
    /*!
     * Set the thickness of the solid
     * \param depth thickness of the solid in model units.
     */
    void setDepth ( double depth );
      
    /*!
     * \return the solid extrusion depth.
     */
    double depth ( void ) const { return extrusion_.depth_; }

  private:
    // It would be nice if C/C++ had real matrix notation (like Perl!)
    void assign ( gleDouble a[3], double x, double y, double z )
    {
      a[X] = x;
      a[Y] = y;
      a[Z] = z;
    }

    void init ( void );
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
    static int moveToCallback ( FT_Vector* to, Solid* solid );
    static int lineToCallback ( FT_Vector* to, Solid* solid );
    static int conicToCallback ( FT_Vector* control, FT_Vector* to, Solid* solid );
    static int cubicToCallback ( FT_Vector* control1, FT_Vector* control2,
				 FT_Vector* to, Solid* solid );
  };
#endif /* OGLFT_NO_SOLID */
  //! This is the base class of the raster styles: bitmap, grayscale and
  //! translucent.
  /*!
   * In the raster styles, FreeType's rasterizer is used to generate raster
   * images of each glyph.
   */
  class Raster : public Face {
  protected:
    //! Raster glyph can be rotated in the Z plane (in addition to the string
    //! rotation).
    GLfloat character_rotation_z_;
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Raster ( const char* filename, float point_size = 12, FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Raster ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
    /*!
     * The destructor doesn't do anything in particular.
     */
    virtual ~Raster ( void );
    /*!
     * Set the individual character rotation in the Z direction.
     * \param character_rotation_z angle in degrees of Z rotation.
     */
    void setCharacterRotationZ ( GLfloat character_rotation_z );
    /*!
     * \return the character rotation in the Z direction.
     */
    GLfloat characterRotationZ ( void ) const { return character_rotation_z_; }

    /*!
     * \return the height (i.e., line spacing) at the current character size.
     */
    double height ( void ) const;

    /*!
     * Implement measuring a character in a raster face.
     * \param c the (latin1) character to measure
     * \return the bounding box of c.
     */
    BBox measure ( unsigned char c );
#ifndef OGLFT_NO_QT
    /*!
     * Implement measuring a character in a raster face.
     * \param c the (UNICODE) character to measure
     * \return the bounding box of c.
     */
    BBox measure ( const QChar c );
#endif /* OGLFT_NO_QT */
    /*!
     * Measure a string of characters. Note: currently, this merely
     * calls Face's measure routine.
     * \param s string of (latin1) characters to measure
     * \return the bounding box of s.
     */
    BBox measure ( const char* s ) { return Face::measure( s ); }
#ifndef OGLFT_NO_QT
    /*!
     * Implement measuring a formatted number
     * \param format the format string
     * \param number to value to format
     * \return the bounding box of the formatted number
     */
    BBox measure ( const QString& format, double number )
    { return Face::measure( format, number ); }
#endif /* OGLFT_NO_QT */

  private:
    void init ( void );
    GLuint compileGlyph ( FT_Face face, FT_UInt glyph_index );
    void setCharSize ( void );
    void setRotationOffset ( void );
    void clearCaches ( void );
  };

  //! Render text as a monochrome raster image.
  /*!
   * \image html monochrome_class.png
   * This is more or less the standard way in which text is intended to
   * be rendered in OpenGL. It uses the \c glBitmap call to draw a sequence
   * of monochrome bitmaps. Since FreeType is capable of rotating glyphs
   * created from faces based on vector outlines, you can rotate (in the Z plane)
   * both the text string as well as the individual characters in the string.
   *
   * Note: you \em must call
   * \code
   * glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   * \endcode
   * before drawing in order for monochrome glyphs to be rendered properly.
   *
   * Another note: It is helpful to have the option
   * \c GL_RASTER_POSITION_UNCLIPPED_IBM available if you intend to draw text
   * at MODELVIEW based positions, otherwise if the initial text position is off
   * the screen, the entire image is clipped.
   */
  class Monochrome : public Raster {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Monochrome ( const char* filename, float point_size = 12,
		 FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Monochrome ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
    /*!
     * The destructor doesn't do anything in particular.
     */
    ~Monochrome ( void );
  private:
    GLubyte* invertBitmap ( const FT_Bitmap& bitmap );
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
  };

  //! Render text as a grayscale raster image.
  /*!
   * \image html grayscale_class.png
   * The Grayscale style is similar to the Monochrome style. FreeType is used
   * to rasterize a glyph and this is then drawn on the screen using
   * \c glDrawPixels. The FreeType rasterization is done in anti-aliased mode.
   * When Grayscale draws the glyph image, the resulting text is blended
   * smoothly from the foreground color to the background color. The background
   * of the glyph is opaque, so this style works best over a solid background.
   *
   * Note: you \em must call
   * \code
   * glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   * \endcode
   * before drawing in order for grayscale glyphs to be rendered properly.
   *
   * Another note: It is helpful to have the option
   * \c GL_RASTER_POSITION_UNCLIPPED_IBM available if you intend to draw text
   * at MODELVIEW based positions, otherwise if the initial text position is off
   * the screen, the entire image is clipped.
   */
  class Grayscale : public Raster {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Grayscale ( const char* filename, float point_size = 12,
		FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Grayscale ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );
    /*!
     * The destructor doesn't do anything in particular.
     */
    ~Grayscale ( void );
  private:
    GLubyte* invertPixmap ( const FT_Bitmap& bitmap );
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
  };

  //! Render text as a translucent raster image.
  /*!
   * \image html translucent_class.png
   * The Translucent style is similar to the Grayscale style. FreeType is used
   * to rasterize a glyph and this is then drawn on the screen using
   * \c glDrawPixels. The FreeType rasterization is done in anti-aliased mode.
   * When Translucent draws the glyph image, the grayscale levels provided
   * by FreeType are used as Alpha values in the raster image. This allows
   * the glyphs to be smoothly blended into complicated backgrounds.
   *
   * Note: you \em must call
   * \code
   * glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   * \endcode
   * before drawing in order for translucent glyphs to be rendered properly.
   * Additionally, you need to activate blending in order to achieve the
   * translucent effect:
   * \code
   * glEnable( GL_BLEND );
   * glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   * \endcode
   *
   * Another note: It is helpful to have the option
   * \c GL_RASTER_POSITION_UNCLIPPED_IBM available if you intend to draw text
   * at MODELVIEW based positions, otherwise if the initial text position is off
   * the screen, the entire image is clipped.
   */
  class Translucent : public Raster {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Translucent ( const char* filename, float point_size = 12,
		  FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Translucent ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * The destructor doesn't do anything in particular.
     */
    ~Translucent ( void );

  private:
    GLubyte* invertPixmapWithAlpha ( const FT_Bitmap& bitmap );
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
  };

  //! This is the base class of the texture style.
  class Texture : public Face {
  protected:
    //! Angle of rotation of characters relative to text orientation.
    struct {
      bool active_; //!< Is character rotation non-zero? (faster than checking all
		    //!< the other values.)
      GLfloat x_,   //!< Angle of rotation in the X direction.
	y_,	    //!< Angle of rotation in the Y direction.
	z_;	    //!< Angle of rotation in the Z direction.
    } character_rotation_;

    /*!
     * The textured glyphs need a little bit more infrastructure to draw
     * since we have to remember the size of the texture object itself
     * (at least implicitly). Also, we don't want to create any more
     * texture objects than we have to, so they are always cached.
     */
    struct TextureInfo {
      GLuint texture_name_;  //!< A bound texture name is an integer in OpenGL.
      FT_Int left_bearing_,  //!< The left bearing of the transformed glyph.
	bottom_bearing_;     //!< The bottom bearing of the transformed glyph.
      int width_,	     //!< The 2**l width of the texture.
	height_;	     //!< The 2**m height of the texture.
      GLfloat texture_s_,    //!< The fraction of the texture width occupied
			     //!< by the glyph.
	texture_t_;	     //!< The fraction of the texture height occupied
                             //!< by the glyph.
      FT_Vector advance_;    //!< The advance vector of the transformed glyph.
    };

    //! Type of the cache of defined glyph to texture objects mapping.
    typedef std::map< FT_UInt, TextureInfo > GlyphTexObjs;

    //! A convenience definition of the iterator over the glyph to texture
    //! object map.
    typedef GlyphTexObjs::const_iterator GTOCI;

    //! A convenience definition of the iterator over the glyph to texture
    //! object map.
    typedef GlyphTexObjs::iterator GTOI;

    //! Cache of defined glyph texture objects.
    GlyphTexObjs glyph_texobjs_;

  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Texture ( const char* filename, float point_size = 12,
	      FT_UInt resolution = 100 );

    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    Texture ( FT_Face face, float point_size = 12, FT_UInt resolution = 100 );

    /*!
     * The texture destructor doesn't really do anything.
     */
    virtual ~Texture ( void );
    /*!
     * Set the individual character rotation in the X direction.
     * \param character_rotation_x angle in degrees of X rotation.
     */
    void setCharacterRotationX ( GLfloat character_rotation_x );

    /*!
     * Set the individual character rotation in the Y direction.
     * \param character_rotation_y angle in degrees of Y rotation.
     */
    void setCharacterRotationY ( GLfloat character_rotation_y );

    /*!
     * Set the individual character rotation in the Z direction.
     * \param character_rotation_z angle in degrees of Z rotation.
     */
    void setCharacterRotationZ ( GLfloat character_rotation_z );

    /*!
     * \return the character rotation in the X direction.
     */
    GLfloat characterRotationX ( void ) const { return character_rotation_.x_; }

    /*!
     * \return the character rotation in the Y direction.
     */
    GLfloat characterRotationY ( void ) const { return character_rotation_.y_; }

    /*!
     * \return the character rotation in the Z direction.
     */
    GLfloat characterRotationZ ( void ) const { return character_rotation_.z_; }

    /*!
     * \return the height (i.e., line spacing) at the current character size.
     */
    double height ( void ) const;

    /*!
     * Implement measuring a character in a texture face.
     * \param c the (latin1) character to measure
     * \return the bounding box of c.
     */
    BBox measure ( unsigned char c );
#ifndef OGLFT_NO_QT
    /*!
     * Implement measuring a character in a texture face.
     * \param c the (UNICODE) character to measure
     * \return the bounding box of c.
     */
    BBox measure ( const QChar c );
#endif /* OGLFT_NO_QT */
    /*!
     * Measure a string of characters. Note: currently, this merely
     * calls Face's measure routine.
     * \param s string of (latin1) characters to measure
     * \return the bounding box of s.
     */
    BBox measure ( const char* s ) { return Face::measure( s ); }
#ifndef OGLFT_NO_QT
    /*!
     * Implement measuring a formatted number
     * \param format the format string
     * \param number to value to format
     * \return the bounding box of the formatted number
     */
    BBox measure ( const QString& format, double number )
    { return Face::measure( format, number ); }
#endif /* OGLFT_NO_QT */

  protected:
    /*!
     * OpenGL texture maps have to be a power of 2 in width and height (including
     * apparently 1 = 2**0 ). This function returns the next higher power of
     * 2 of the argument. If the argument is already a power of 2, you just
     * get that back.
     * \param a width or height of an image.
     * \return value of a rounded to nearest, higher power of 2.
     */
    unsigned int nearestPowerCeil ( unsigned int a );
    /*!
     * This is all that distinguishes the various texture styles. Each subclass
     * defines this method as appropriate. Once the texture is bound, it
     * is rendered the same in all cases.
     * \param face FT_Face containing the glyph to render.
     * \param glyph_index index of glyph in face.
     */
    virtual void bindTexture ( FT_Face face, FT_UInt glyph_index ) = 0;

  private:
    void init ( void );
    void setCharSize ( void );
    void setRotationOffset ( void );
    GLuint compileGlyph ( FT_Face face, FT_UInt glyph_index );
    void renderGlyph ( FT_Face face, FT_UInt glyph_index );
    void clearCaches ( void );
  };

  //! Render text as texture mapped monochrome quads.
  /*!
   * \image html texture_monochrome_class.png
   * This style is similar to the Monochrome raster style, except instead
   * of using \c glBitmap to draw the raster image, the image is used
   * as a texture map on a quad. If drawing is confined to the Z plane,
   * then you will see no difference between this style and Monochrome.
   * However, because the quad is a 3D object, it can be transformed
   * by the usual modeling operations; so, texture mapped glyphs can be
   * rotated in the X and Y directions as well as Z direction. Also,
   * if the viewing (or modeling) transformation has a non-unity scale or
   * shear, the glyphs will also be scaled or sheared (unlike the raster
   * styles). Also, there is no problem with clipping glyphs which lie
   * off the screen; texture mapped quads are properly clipped to the
   * screen boundary.
   *
   * If this is not convincing enough, the performance of texture mapped
   * glyphs is generally as good as or better than the equivalent
   * raster style (especially with hardware texture acceleration). However,
   * they do take up more memory space.
   *
   * Note: you \em must call
   * \code
   * glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   * \endcode
   * before drawing in order for textured glyphs to be rendered properly.
   */
  class MonochromeTexture : public Texture {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    MonochromeTexture ( const char* filename, float point_size = 12,
			FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    MonochromeTexture ( FT_Face face, float point_size = 12,
			FT_UInt resolution = 100 );
    /*!
     * The monochrome texture destructor doesn't really do anything.
     */
    ~MonochromeTexture ( void );
  private:
    GLubyte* invertBitmap ( const FT_Bitmap& bitmap, int* width, int* height );
    void bindTexture ( FT_Face face, FT_UInt glyph_index );
  };

  //! Render text as texture mapped grayscale quads.
  /*!
   * \image html texture_grayscale_class.png
   * This style is similar to the Grayscale raster style, except instead
   * of using \c glDrawPixels to draw the raster image, the image is used
   * as a texture map on a quad. If drawing is confined to the Z plane,
   * then you will see no difference between this style and Grayscale.
   * However, because the quad is a 3D object, it can be transformed
   * by the usual modeling operations; so, texture mapped glyphs can be
   * rotated in the X and Y directions as well as Z direction. Also,
   * if the viewing (or modeling) transformation has a non-unity scale or
   * shear, the glyphs will also be scaled or sheared (unlike the raster
   * styles). Also, there is no problem with clipping glyphs which lie
   * off the screen; texture mapped quads are properly clipped to the
   * screen boundary.
   *
   * If this is not convincing enough, the performance of texture mapped
   * glyphs is generally as good as or better than the equivalent
   * raster style (especially with hardware texture acceleration). However,
   * they do consume more memory space.
   *
   * Note: you \em must call
   * \code
   * glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   * \endcode
   * before drawing in order for textured glyphs to be rendered properly.
   */
  class GrayscaleTexture : public Texture {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    GrayscaleTexture ( const char* filename, float point_size = 12,
		       FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    GrayscaleTexture ( FT_Face face, float point_size = 12,
		       FT_UInt resolution = 100 );
    /*!
     * The grayscale texture destructor doesn't really do anything.
     */
    ~GrayscaleTexture ( void );
  private:
    GLubyte* invertPixmap ( const FT_Bitmap& bitmap, int* width, int* height );
    void bindTexture ( FT_Face face, FT_UInt glyph_index );
  };

  //! Render text as texture mapped translucent quads.
  /*!
   * \image html texture_translucent_class.png
   * This style is similar to the Translucent raster style, except instead
   * of using \c glDrawPixels to draw the raster image, the image is used
   * as a texture map on a quad. If drawing is confined to the Z plane,
   * then you will see no difference between this style and Translucent.
   * However, because the quad is a 3D object, it can be transformed
   * by the usual modeling operations; so, texture mapped glyphs can be
   * rotated in the X and Y directions as well as Z direction. Also,
   * if the viewing (or modeling) transformation has a non-unity scale or
   * shear, the glyphs will also be scaled or sheared (unlike the raster
   * styles). Also, there is no problem with clipping glyphs which lie
   * off the screen; texture mapped quads are properly clipped to the
   * screen boundary.
   *
   * If this is not convincing enough, the performance of texture mapped
   * glyphs is generally as good as or better than the equivalent
   * raster style (especially with hardware texture acceleration). However,
   * they do consume more memory space.
   *
   * Note: you \em must call
   * \code
   * glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   * \endcode
   * before drawing in order for textured glyphs to be rendered properly.
   * Additionally, you need to activate blending in order to achieve the
   * translucent effect:
   * \code
   * glEnable( GL_BLEND );
   * glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   * \endcode
   */
  class TranslucentTexture : public Texture {
  public:
    /*!
     * \param filename the filename which contains the font face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    TranslucentTexture ( const char* filename, float point_size = 12,
			 FT_UInt resolution = 100 );
    /*!
     * \param face open FreeType FT_Face.
     * \param point_size the initial point size of the font to generate. A point
     * is essentially 1/72th of an inch. Defaults to 12.
     * \param resolution the pixel density of the display in dots per inch (DPI).
     * Defaults to 100 DPI.
     */
    TranslucentTexture ( FT_Face face, float point_size = 12,
			 FT_UInt resolution = 100 );
    /*!
     * The translucent texture destructor doesn't really do anything.
     */
    ~TranslucentTexture ( void );
  private:
    GLubyte* invertPixmap ( const FT_Bitmap& bitmap, int* width, int* height );
    void bindTexture ( FT_Face face, FT_UInt glyph_index );
  };
} // Close OGLFT namespace
#endif /* OGLFT_H */
