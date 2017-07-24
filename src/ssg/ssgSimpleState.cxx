
#include "ssgLocal.h"

static ssgSimpleState *ssgCurrentState = NULL ;
static ssgSimpleState *ssgBasicState   = NULL ;

void _ssgInitState ()
{
  ssgCurrentState = new ssgSimpleState ( 1 ) ;
  ssgBasicState   = new ssgSimpleState ( 0 ) ;
  ssgCurrentState -> force () ;

  ssgBasicState -> dont_care      = 0 ;
  ssgBasicState -> texture_handle = 0 ;
  ssgBasicState -> colour_material_mode = GL_AMBIENT_AND_DIFFUSE ;
  sgSetVec4 ( ssgBasicState -> specular_colour, 1.0f, 1.0f, 1.0f, 1.0f ) ;
  sgSetVec4 ( ssgBasicState -> emission_colour, 0.0f, 0.0f, 0.0f, 1.0f ) ;
  sgSetVec4 ( ssgBasicState ->  ambient_colour, 1.0f, 1.0f, 1.0f, 1.0f ) ;
  sgSetVec4 ( ssgBasicState ->  diffuse_colour, 1.0f, 1.0f, 1.0f, 1.0f ) ;
  ssgBasicState -> shade_model = GL_SMOOTH ;
  ssgBasicState -> shininess   = 0.0f ;
  ssgBasicState -> alpha_clamp = 0.01f ;
}


void _ssgForceLineState ()
{
  ssgCurrentState->enables &= ~((1<<SSG_GL_TEXTURE_EN) |
                                (1<<SSG_GL_LIGHTING_EN)|
                                (1<<SSG_GL_COLOR_MATERIAL_EN)) ;
  glDisable ( GL_TEXTURE_2D ) ;
  glDisable ( GL_COLOR_MATERIAL ) ;
  glDisable ( GL_LIGHTING  ) ;
  glDisable ( GL_DEPTH_TEST ) ;
}

void ssgForceBasicState ()
{
  if ( ssgBasicState == NULL )
    _ssgInitState () ;

  ssgBasicState -> force () ;
}


ssgSimpleState::ssgSimpleState ( int /* I_am_currstate */ )
{
  type |= SSG_TYPE_SIMPLESTATE ;

  dont_care = 0xFFFFFFFF ;
  disable ( GL_TEXTURE_2D  ) ;
  enable  ( GL_CULL_FACE   ) ;
  enable  ( GL_COLOR_MATERIAL ) ;
  enable  ( GL_LIGHTING ) ;
  disable ( GL_BLEND ) ;
  enable  ( GL_ALPHA_TEST ) ;
  setShadeModel ( GL_SMOOTH ) ;
}

ssgSimpleState::ssgSimpleState (void)
{
  type |= SSG_TYPE_SIMPLESTATE ;

  dont_care = 0xFFFFFFFF ;
}

ssgSimpleState::~ssgSimpleState (void) {}

void ssgSimpleState::apply (void)
{
  if ( ~ dont_care & ( (1<<SSG_GL_COLOR_MATERIAL) |
                       (1<<SSG_GL_DIFFUSE       ) |
                       (1<<SSG_GL_AMBIENT       ) |
                       (1<<SSG_GL_SPECULAR      ) |
                       (1<<SSG_GL_EMISSION      ) |
                       (1<<SSG_GL_SHININESS     ) ) )
  {
    int switched_modes = FALSE ;

    if ( ~ dont_care & (1<<SSG_GL_COLOR_MATERIAL ) &&
      ssgCurrentState -> colour_material_mode != colour_material_mode )
    {
      glColorMaterial ( GL_FRONT_AND_BACK, (GLenum) colour_material_mode ) ;
      ssgCurrentState -> colour_material_mode = colour_material_mode ;
      switched_modes = TRUE ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_SHININESS) ) &&
      ssgCurrentState -> shininess != shininess )
    {
      glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess ) ;
      ssgCurrentState -> shininess = shininess ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_SPECULAR) ) &&
      ( switched_modes ||
        ! sgEqualVec3 ( ssgCurrentState -> specular_colour, specular_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_colour ) ;
      sgCopyVec3 ( ssgCurrentState -> specular_colour, specular_colour ) ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_EMISSION) ) &&
      ( switched_modes ||
      ! sgEqualVec3 ( ssgCurrentState -> emission_colour, emission_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, emission_colour ) ;
      sgCopyVec3 ( ssgCurrentState -> emission_colour, emission_colour ) ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_AMBIENT) ) &&
      ( switched_modes ||
      ! sgEqualVec3 ( ssgCurrentState -> ambient_colour, ambient_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT, ambient_colour ) ;
      sgCopyVec3 ( ssgCurrentState -> ambient_colour, ambient_colour ) ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_DIFFUSE) ) &&
      ( switched_modes ||
      ! sgEqualVec4 ( ssgCurrentState -> diffuse_colour, diffuse_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_colour ) ;
      sgCopyVec4 ( ssgCurrentState -> diffuse_colour, diffuse_colour ) ;
    }
  }

  int turn_on  = ~dont_care &  enables & ~ssgCurrentState->enables & 0x3F ;
  int turn_off = ~dont_care & ~enables &  ssgCurrentState->enables & 0x3F ;

  (*(__ssgEnableTable [turn_on ]))() ;
  (*(__ssgDisableTable[turn_off]))() ;

  ssgCurrentState -> enables |=  turn_on  ;
  ssgCurrentState -> enables &= ~turn_off ;

  if ( ( ~ dont_care & (1<<SSG_GL_TEXTURE) ) && 
    ssgCurrentState -> texture_handle != texture_handle )
  {
    stats_bind_textures++ ;
#ifdef GL_VERSION_1_1
    glBindTexture ( GL_TEXTURE_2D, texture_handle ) ;
#else
    /* For ancient SGI machines */
    glBindTextureEXT ( GL_TEXTURE_2D, texture_handle ) ;
#endif
    ssgCurrentState -> texture_handle = texture_handle ;
  }

  if ( ( ~ dont_care & (1<<SSG_GL_SHADE_MODEL) ) &&
    ssgCurrentState -> shade_model != shade_model )
  {
    glShadeModel ( shade_model ) ;
    ssgCurrentState -> shade_model = shade_model ;
  }

  if ( ( ~ dont_care & (1<<SSG_GL_ALPHA_TEST) ) &&
    ssgCurrentState -> alpha_clamp != alpha_clamp )
  {
    glAlphaFunc ( GL_GREATER, alpha_clamp ) ;
    ssgCurrentState -> alpha_clamp = alpha_clamp ;
  }
}

void ssgSimpleState::force (void)
{
/*
  glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess ) ;
  ssgCurrentState -> shininess = shininess ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_colour ) ;
  sgCopyVec3 ( ssgCurrentState -> specular_colour, specular_colour ) ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, emission_colour ) ;
  sgCopyVec3 ( ssgCurrentState -> emission_colour, emission_colour ) ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT, ambient_colour ) ;
  sgCopyVec3 ( ssgCurrentState -> ambient_colour, ambient_colour ) ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_colour ) ;
  sgCopyVec4 ( ssgCurrentState -> diffuse_colour, diffuse_colour ) ;
*/
  if ( ~ dont_care & ( (1<<SSG_GL_COLOR_MATERIAL ) |
                       (1<<SSG_GL_DIFFUSE        ) |
                       (1<<SSG_GL_AMBIENT        ) |
                       (1<<SSG_GL_SPECULAR       ) |
                       (1<<SSG_GL_EMISSION       ) |
                       (1<<SSG_GL_SHININESS      ) ) )
  {
    if ( ~ dont_care & (1<<SSG_GL_COLOR_MATERIAL ) )
    {
      glColorMaterial ( GL_FRONT_AND_BACK, (GLenum) colour_material_mode ) ;
      ssgCurrentState -> colour_material_mode = colour_material_mode ;
    }

    if ( ~ dont_care & (1<<SSG_GL_SHININESS ) )
    {
      glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess ) ;
      ssgCurrentState -> shininess = shininess ;
    }

    if ( ~ dont_care & (1<<SSG_GL_DIFFUSE ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_colour ) ;
      sgCopyVec4 ( ssgCurrentState -> diffuse_colour, diffuse_colour ) ;
    }

    if ( ~ dont_care & (1<<SSG_GL_EMISSION ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, emission_colour ) ;
      sgCopyVec3 ( ssgCurrentState -> emission_colour, emission_colour ) ;
    }

    if ( ~ dont_care & (1<<SSG_GL_AMBIENT ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT, ambient_colour ) ;
      sgCopyVec3 ( ssgCurrentState -> ambient_colour, ambient_colour ) ;
    }

    if ( ~ dont_care & (1<<SSG_GL_SPECULAR ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_colour ) ;
      sgCopyVec3 ( ssgCurrentState -> specular_colour, specular_colour ) ;
    }
  }

  int turn_on  = ~dont_care &  enables & 0x3F ;
  int turn_off = ~dont_care & ~enables & 0x3F ;

  (*(__ssgEnableTable [turn_on ]))() ;
  (*(__ssgDisableTable[turn_off]))() ;

  ssgCurrentState -> enables |=  turn_on  ;
  ssgCurrentState -> enables &= ~turn_off ;

  if ( ~ dont_care & (1<<SSG_GL_TEXTURE ) )
  {
    stats_bind_textures++ ;
#ifdef GL_VERSION_1_1
    glBindTexture ( GL_TEXTURE_2D, texture_handle ) ;
#else
    /* For ancient SGI machines */
    glBindTextureEXT ( GL_TEXTURE_2D, texture_handle ) ;
#endif
    ssgCurrentState -> texture_handle = texture_handle ;
  }

  if ( ~ dont_care & (1<<SSG_GL_SHADE_MODEL ) )
  {
    glShadeModel ( shade_model ) ;
    ssgCurrentState -> shade_model = shade_model ;
  }

  if ( ~ dont_care & (1<<SSG_GL_ALPHA_TEST ) )
  {
    glAlphaFunc ( GL_GREATER, alpha_clamp ) ;
    ssgCurrentState -> alpha_clamp = alpha_clamp ;
  }
}

void ssgSimpleState::print ( FILE *fd, char *indent )
{
  ssgState::print ( fd, indent ) ;
}



int ssgSimpleState::isEnabled ( GLenum mode )
{
  switch ( mode )
  {
    case GL_TEXTURE_2D        :
      return enables & (1<<SSG_GL_TEXTURE_EN) ;

    case GL_CULL_FACE      :
      return enables & (1<<SSG_GL_CULL_FACE_EN) ;

    case GL_COLOR_MATERIAL :
      return enables & (1<<SSG_GL_COLOR_MATERIAL_EN) ;

    case GL_LIGHTING :
      return enables & (1<<SSG_GL_LIGHTING_EN) ;

    case GL_BLEND          :
      return enables & (1<<SSG_GL_BLEND_EN) ;

    case GL_ALPHA_TEST     :
      return enables & (1<<SSG_GL_ALPHA_TEST_EN) ;

    default :
      return FALSE ;
  }
}



void ssgSimpleState::disable ( GLenum mode )
{
  switch ( mode )
  {
    case GL_TEXTURE_2D        :
      enables &= ~ (1<<SSG_GL_TEXTURE_EN) ;
      care_about ( SSG_GL_TEXTURE_EN ) ;
      break ;

    case GL_CULL_FACE      :
      enables &= ~ (1<<SSG_GL_CULL_FACE_EN) ;
      care_about ( SSG_GL_CULL_FACE_EN ) ;
      break ;

    case GL_COLOR_MATERIAL :
      enables &= ~ (1<<SSG_GL_COLOR_MATERIAL_EN) ;
      care_about ( SSG_GL_COLOR_MATERIAL_EN ) ;
      break ;

    case GL_LIGHTING :
      enables &= ~ (1<<SSG_GL_LIGHTING_EN) ;
      care_about ( SSG_GL_LIGHTING_EN ) ;
      break ;

    case GL_BLEND          :
      enables &= ~ (1<<SSG_GL_BLEND_EN) ;
      care_about ( SSG_GL_BLEND_EN ) ;
      break ;

    case GL_ALPHA_TEST     :
      enables &= ~ (1<<SSG_GL_ALPHA_TEST_EN) ;
      care_about ( SSG_GL_ALPHA_TEST_EN ) ;
      break ;

    default :
      fprintf ( stderr,
	     "Illegal mode passed to ssgSimpleState::disable(%d)\n",
			       mode ) ;
      break ; 
  }
}

void ssgSimpleState::enable  ( GLenum mode )
{
  switch ( mode )
  {
    case GL_TEXTURE_2D        :
      enables |=  (1<<SSG_GL_TEXTURE_EN) ;
      care_about ( SSG_GL_TEXTURE_EN ) ;
      break ;

    case GL_CULL_FACE      :
      enables |=  (1<<SSG_GL_CULL_FACE_EN) ;
      care_about ( SSG_GL_CULL_FACE_EN ) ;
      break ;

    case GL_COLOR_MATERIAL :
      enables |=  (1<<SSG_GL_COLOR_MATERIAL_EN) ;
      care_about ( SSG_GL_COLOR_MATERIAL_EN ) ;
      break ;

    case GL_BLEND          :
      enables |=  (1<<SSG_GL_BLEND_EN) ;
      care_about ( SSG_GL_BLEND_EN ) ;
      break ;

    case GL_ALPHA_TEST     :
      enables |=  (1<<SSG_GL_ALPHA_TEST_EN) ;
      care_about ( SSG_GL_ALPHA_TEST_EN ) ;
      break ;

    case GL_LIGHTING :
      enables |= (1<<SSG_GL_LIGHTING_EN) ;
      care_about ( SSG_GL_LIGHTING_EN ) ;
      break ;

    default :
      fprintf ( stderr,
	     "Illegal mode passed to ssgSimpleState::enable(%d)\n",
			       mode ) ;
      break ; 
  }
}


int ssgSimpleState::load ( FILE *fd )
{
  _ssgReadInt   ( fd, & dont_care            ) ;
  _ssgReadInt   ( fd, & enables              ) ;
  _ssgReadUInt  ( fd, & texture_handle       ) ;
  _ssgReadInt   ( fd, & colour_material_mode ) ;
  _ssgReadVec4  ( fd, specular_colour        ) ;
  _ssgReadVec4  ( fd, emission_colour        ) ;
  _ssgReadVec4  ( fd, ambient_colour         ) ;
  _ssgReadVec4  ( fd, diffuse_colour         ) ;
  _ssgReadInt   ( fd, (int *) (& shade_model)) ;
  _ssgReadFloat ( fd, & shininess            ) ;
  _ssgReadFloat ( fd, & alpha_clamp          ) ;

  return ssgState::load(fd) ;
}


int ssgSimpleState::save ( FILE *fd )
{
  _ssgWriteInt   ( fd, dont_care            ) ;
  _ssgWriteInt   ( fd, enables              ) ;
  _ssgWriteUInt  ( fd, texture_handle       ) ;
  _ssgWriteInt   ( fd, colour_material_mode ) ;
  _ssgWriteVec4  ( fd, specular_colour      ) ;
  _ssgWriteVec4  ( fd, emission_colour      ) ;
  _ssgWriteVec4  ( fd, ambient_colour       ) ;
  _ssgWriteVec4  ( fd, diffuse_colour       ) ;
  _ssgWriteInt   ( fd, (int) shade_model    ) ;
  _ssgWriteFloat ( fd, shininess            ) ;
  _ssgWriteFloat ( fd, alpha_clamp          ) ;

  return ssgState::save(fd) ;
}



