
#include "ssgLocal.h"

sgMat4 _ssgOpenGLAxisSwapMatrix =
{
  {  1.0f,  0.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f, -1.0f,  0.0f },
  {  0.0f,  1.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f,  0.0f,  1.0f }
} ;

sgVec3 _ssgVertex000   = { 0.0f, 0.0f, 0.0f } ;
sgVec4 _ssgColourWhite = { 1.0f, 1.0f, 1.0f, 1.0f } ;
sgVec3 _ssgNormalUp    = { 0.0f, 0.0f, 1.0f } ;
sgVec2 _ssgTexCoord00  = { 0.0f, 0.0f } ;

int        _ssgOrthographic     = FALSE ;
sgFrustum *_ssgCurrentFrustum   = NULL ;
sgMat4     _ssgCurrentCameraMatrix ;
int        _ssgCurrentCullFace  = TRUE ;
int        _ssgOverrideTexture  = FALSE ;
int        _ssgOverrideCullface = FALSE ;
int        _ssgIsHotTest        = FALSE ;

ssgLight   _ssgLights [ 8 ] ;

int         _ssgFrameCounter = 0 ;

int ssgGetFrameCounter () { return _ssgFrameCounter ; }
void ssgSetFrameCounter ( int fc ) { _ssgFrameCounter = fc ; }

char *ssgGetVersion ()
{
#ifdef VERSION
  return VERSION ;
#else
  return "Unknown" ;
#endif
}

void ssgDelete ( ssgBranch *br )
{
  if ( br == NULL )
    return ;

  br -> removeAllKids () ;
  delete br ;
}

ssgLight *ssgGetLight ( int i )
{
  return &_ssgLights [ i ] ;
}

void ssgGetCameraPosition ( sgVec3 pos )
{
  sgCopyVec3 ( pos, _ssgCurrentCameraMatrix [ 3 ] ) ;
}

void ssgOverrideTexture ( int on_off )
{
  _ssgOverrideTexture = on_off ;
}

void ssgOverrideCullface ( int on_off )
{
  _ssgOverrideCullface = on_off ;
}

void ssgInit ()
{
  ssgTexturePath ( "." ) ;
  ssgModelPath   ( "." ) ;

  _ssgCurrentFrustum = new sgFrustum ;
  _ssgCurrentFrustum -> setNearFar ( 1.0, 10000.0 ) ;
  _ssgCurrentFrustum -> setFOV     ( 60.0, 45.0 ) ;

  sgMakeIdentMat4 ( _ssgCurrentCameraMatrix ) ;

  _ssgLights [ 0 ] . setID ( 0 ) ;
  _ssgLights [ 0 ] . on    () ;

  for ( int i = 1 ; i < 8 ; i++ )
  {
    _ssgLights [ i ] . setID ( i ) ;
    _ssgLights [ i ] . off   () ;
  }

  if ( ! _ssgOverrideCullface )
  {
    _ssgCurrentCullFace = TRUE ;
    glEnable ( GL_CULL_FACE ) ;
  }
 
  _ssgInitState () ;
}


void ssgGetNearFar ( float *n, float *f )
{
  _ssgCurrentFrustum -> getNearFar ( n, f ) ;
}


void ssgGetFOV ( float *w, float *h )
{
  _ssgCurrentFrustum -> getFOV ( w, h ) ;
}


void ssgSetNearFar ( float n, float f )
{
  _ssgCurrentFrustum -> setNearFar ( n, f ) ;
}


void ssgGetOrtho ( float *w, float *h )
{
  if ( w != NULL )
    *w = _ssgCurrentFrustum -> getRight() - _ssgCurrentFrustum -> getLeft() ;

  if ( h != NULL )
    *h = _ssgCurrentFrustum -> getTop() - _ssgCurrentFrustum -> getBot() ;
}


void ssgSetOrtho ( float w, float h )
{
  _ssgOrthographic = TRUE ;
  _ssgCurrentFrustum -> setFrustum ( -w/2, w/2, -h/2, h/2, 
     _ssgCurrentFrustum -> getNear (),
     _ssgCurrentFrustum -> getFar  () ) ;
}



void ssgSetFOV ( float w, float h )
{
  _ssgOrthographic = FALSE ;
  _ssgCurrentFrustum -> setFOV ( w, h ) ;
}


void ssgSetCamera ( sgMat4 mat )
{
  sgMat4 viewmat ;


  sgTransposeNegateMat4 ( viewmat, mat ) ;

  sgCopyMat4      ( _ssgCurrentCameraMatrix, _ssgOpenGLAxisSwapMatrix ) ;
  sgPreMultMat4   ( _ssgCurrentCameraMatrix, viewmat ) ;
}


void ssgSetCamera ( sgCoord *coord )
{
  sgMat4 viewmat, mat ;

  sgMakeCoordMat4 ( mat, coord ) ;
  sgTransposeNegateMat4 ( viewmat, mat ) ;

  sgCopyMat4      ( _ssgCurrentCameraMatrix, _ssgOpenGLAxisSwapMatrix ) ;
  sgPreMultMat4   ( _ssgCurrentCameraMatrix, viewmat ) ;
}


void ssgLoadProjectionMatrix ()
{
  ssgLoadProjectionMatrix ( _ssgCurrentFrustum ) ;
}

static void pushProjectionMatrix ( sgFrustum *f )
{
  if ( _ssgOrthographic )
    glOrtho   ( f -> getLeft() , f -> getRight(),
		f -> getBot () , f -> getTop  (),
		f -> getNear() , f -> getFar  () ) ;
  else
    glFrustum ( f -> getLeft() , f -> getRight(),
		f -> getBot () , f -> getTop  (),
		f -> getNear() , f -> getFar  () ) ;
}

void ssgLoadProjectionMatrix ( sgFrustum *f )
{
  glLoadIdentity () ;
  pushProjectionMatrix ( f ) ;
}


void ssgGetProjectionMatrix ( sgMat4 dst )
{
  if ( _ssgOrthographic )
  {
    float l =  _ssgCurrentFrustum -> getLeft  () ;
    float r =  _ssgCurrentFrustum -> getRight () ;
    float b =  _ssgCurrentFrustum -> getBot   () ;
    float t =  _ssgCurrentFrustum -> getTop   () ;
    float n =  _ssgCurrentFrustum -> getNear  () ;
    float f =  _ssgCurrentFrustum -> getFar   () ;

    sgMakeIdentMat4 ( dst ) ;
    dst[0][0] =  2.0f / ( r - l ) ;
    dst[1][1] =  2.0f / ( t - b ) ;
    dst[2][2] = -2.0f / ( f - n ) ;

    dst[3][0] = - ( r + l ) / ( r - l ) ;
    dst[3][1] = - ( t + b ) / ( t - b ) ;
    dst[3][2] = - ( f + n ) / ( f - n ) ;
  }
  else
    _ssgCurrentFrustum -> getMat4 ( dst ) ;
}


void ssgGetModelviewMatrix ( sgMat4 dst )
{
  sgCopyMat4 ( dst, _ssgCurrentCameraMatrix ) ;
}


void ssgLoadModelviewMatrix ()
{
  ssgLoadModelviewMatrix ( _ssgCurrentCameraMatrix ) ;
}

void ssgLoadModelviewMatrix ( sgMat4 mat )
{
  glLoadMatrixf ( (float *) mat ) ;
}


void ssgCullAndPick ( ssgRoot *r, sgVec2 botleft, sgVec2 topright )
{
  if ( _ssgCurrentFrustum == NULL )
  {
    fprintf ( stderr, "You forgot to call ssgInit()!\n" ) ;
    exit ( 1 ) ;
  }

  ssgForceBasicState () ;

  int w = (int)(topright[0] - botleft[0]) ;
  int h = (int)(topright[1] - botleft[1]) ;

  int x = (int)(botleft[0] + topright[0]) / 2 ;
  int y = (int)(botleft[1] + topright[1]) / 2 ;

  GLint viewport [ 4 ] ;
  glGetIntegerv ( GL_VIEWPORT, viewport ) ;
  glMatrixMode ( GL_PROJECTION ) ;
  glLoadIdentity () ;
  gluPickMatrix ( x, y, w, h, viewport ) ;
  pushProjectionMatrix ( _ssgCurrentFrustum ) ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  int i ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  ssgLoadModelviewMatrix  ( _ssgCurrentCameraMatrix ) ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( ! _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  r -> cull ( _ssgCurrentFrustum, _ssgCurrentCameraMatrix, TRUE ) ;

  _ssgDrawDList () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
}


void ssgCullAndDraw ( ssgRoot *r )
{
  if ( _ssgCurrentFrustum == NULL )
  {
    fprintf ( stderr, "You forgot to call ssgInit()!\n" ) ;
    exit ( 1 ) ;
  }

  _ssgFrameCounter++ ;

  ssgForceBasicState () ;

  glMatrixMode ( GL_PROJECTION ) ;
  ssgLoadProjectionMatrix ( _ssgCurrentFrustum ) ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  int i ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  ssgLoadModelviewMatrix  ( _ssgCurrentCameraMatrix ) ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( ! _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  r -> cull ( _ssgCurrentFrustum, _ssgCurrentCameraMatrix, TRUE ) ;

  _ssgDrawDList () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
}

char *_ssgModelPath = NULL ;
char *_ssgTexturePath = NULL ;

void ssgModelPath ( char *s )
{
  delete _ssgModelPath ;
  _ssgModelPath = new char [ strlen ( s ) + 1 ] ;
  strcpy ( _ssgModelPath, s ) ;
}

void ssgTexturePath ( char *s )
{
  delete _ssgTexturePath ;
  _ssgTexturePath = new char [ strlen ( s ) + 1 ] ;
  strcpy ( _ssgTexturePath, s ) ;
}


char *ssgBase         ::getTypeName (void) { return "ssgBase"          ; }
char *ssgTexture      ::getTypeName (void) { return "ssgTexture"       ; }
char *ssgState        ::getTypeName (void) { return "ssgState"         ; }
char *ssgSimpleState  ::getTypeName (void) { return "ssgSimpleState"   ; }
char *ssgStateSelector::getTypeName (void) { return "ssgStateSelector" ; }
char *ssgEntity       ::getTypeName (void) { return "ssgEntity"        ; }
char *ssgLeaf         ::getTypeName (void) { return "ssgLeaf"          ; }
char *ssgVTable       ::getTypeName (void) { return "ssgVTable"        ; }
char *ssgVtxTable     ::getTypeName (void) { return "ssgVtxTable"      ; }
char *ssgBranch       ::getTypeName (void) { return "ssgBranch"        ; }
char *ssgSelector     ::getTypeName (void) { return "ssgSelector"      ; }
char *ssgRangeSelector::getTypeName (void) { return "ssgRangeSelector" ; }
char *ssgTimedSelector::getTypeName (void) { return "ssgTimedSelector" ; }
char *ssgBaseTransform::getTypeName (void) { return "ssgBaseTransform" ; }
char *ssgTransform    ::getTypeName (void) { return "ssgTransform"     ; }
char *ssgTexTrans     ::getTypeName (void) { return "ssgTexTrans"      ; }
char *ssgCutout       ::getTypeName (void) { return "ssgCutout"        ; }
char *ssgRoot         ::getTypeName (void) { return "ssgRoot"          ; }

void ssgDeRefDelete ( ssgBase *s )
{
  if ( s == NULL ) return ;

  s -> deRef () ;

  if ( s -> getRef () <= 0 )
    delete s ;
}


