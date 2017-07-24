
#include "ssgLocal.h"

static int _ssgInstanceListLength = 0 ;
static ssgBase **_ssgInstanceList = NULL ;
static int _ssgNextInstanceKey = 0 ;

int _ssgGetNextInstanceKey ()
{
  return ++_ssgNextInstanceKey ;
}

ssgBase *_ssgGetFromList ( int key )
{
  return ( _ssgInstanceListLength <= key ) ? (ssgBase *) NULL :
                                             _ssgInstanceList[key] ;
}


void _ssgAddToList ( int key, ssgBase *b )
{
  if ( _ssgInstanceListLength <= key )
  {
    int temp_length = _ssgInstanceListLength ;
    ssgBase **temp = _ssgInstanceList ;
    int new_length = (_ssgInstanceListLength * 2 < key) ? (key + 128) :
                                          (_ssgInstanceListLength * 2) ;

    _ssgInstanceListLength = new_length ;

    _ssgInstanceList = new ssgBase *[ new_length ] ;
    memset ( _ssgInstanceList, 0, new_length * sizeof(ssgBase *) ) ;

    if ( temp_length > 0 )
    {
      memcpy ( _ssgInstanceList, temp, temp_length * sizeof(ssgBase *) ) ;
      delete[] temp ;
    }
  }

  _ssgInstanceList [ key ] = b ;
}


ssgEntity *ssgLoadSSG ( char *fname, ssgHookFunc hookfunc )
{
  delete[] _ssgInstanceList ;
  _ssgInstanceList = NULL ;
  _ssgInstanceListLength = 0 ;

  char filename [ 1024 ] ;

  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;

  FILE *fd = fopen ( filename, "rb" ) ;

  if ( fd == NULL )
  {
    perror ( filename ) ;
    fprintf ( stderr,
      "ssgLoadSSG: Failed to open '%s' for reading\n", filename ) ;
    return NULL ;
  }

  int t ;
  ssgEntity *kid ;

  _ssgReadInt ( fd, & t ) ;

  if ( t == ssgTypeVTable       () ) kid = new ssgVTable       () ; else
  if ( t == ssgTypeVtxTable     () ) kid = new ssgVtxTable     () ; else
  if ( t == ssgTypeBranch       () ) kid = new ssgBranch       () ; else
  if ( t == ssgTypeTransform    () ) kid = new ssgTransform    () ; else
  if ( t == ssgTypeTexTrans     () ) kid = new ssgTexTrans     () ; else
  if ( t == ssgTypeSelector     () ) kid = new ssgSelector     () ; else
  if ( t == ssgTypeRangeSelector() ) kid = new ssgRangeSelector() ; else
  if ( t == ssgTypeTimedSelector() ) kid = new ssgTimedSelector() ; else
  if ( t == ssgTypeCutout       () ) kid = new ssgCutout       () ; else
  if ( t == ssgTypeInvisible    () ) kid = new ssgInvisible    () ; else
  if ( t == ssgTypeRoot         () ) kid = new ssgRoot         () ; else
  {
    fprintf ( stderr,
      "loadSSG: Unrecognised Entity type 0x%08x\n", t ) ;
    return NULL ;
  }

  if ( ! kid -> load ( fd ) )
  {
    fprintf ( stderr,
      "loadSSG: Failed to read child object.\n" ) ;
    return NULL ;
  }

  kid -> recalcBSphere () ;

  fclose ( fd ) ;
  return kid ;
}


void ssgSaveSSG ( char *fname, ssgEntity *ent )
{
  /* Uses the spare field in every entity to make sure
    we don't save the same thing twice */

  _ssgNextInstanceKey = 0 ;
  ent -> zeroSpareRecursive () ;

  char filename [ 1024 ] ;

  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;

  FILE *fd = fopen ( filename, "wb" ) ;

  if ( fd == NULL )
  {
    perror ( filename ) ;
    fprintf ( stderr,
      "ssgSaveSSG: Failed to open '%s' for writing\n", filename ) ;
    return ;
  }

  _ssgWriteInt ( fd, ent->getType() ) ;

  if ( ! ent -> save ( fd ) )
  {
    fprintf ( stderr,
      "ssgSaveSSG: Failed to write child object.\n" ) ;
    return ;
  }

  fclose ( fd ) ;
}


