#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "XmlLoad.h"

static BOOLEAN parse( CHAR* pcData, struct tagXMLDocument *pDoc );

BOOLEAN loadXML( const CHAR *strPath, struct tagXMLDocument *pDoc )
{
    if ( NULL == strPath || NULL == pDoc ) return FALSE;

    int fd = 0;
    if ( ( fd = open( strPath, O_RDONLY ) ) == -1 ) {
        return FALSE;
    }

    // get size and load file 
    UINT uiFileSize = lseek( fd, 0, SEEK_END );
    lseek( fd, 0, SEEK_SET );
    CHAR *pBuff = (CHAR*)malloc(uiFileSize);
    if ( uiFileSize != read( fd, pBuff, uiFileSize ) ) goto err_buffer; 
 
    // skip xml header
    CHAR *pParseBuffer = pBuff;
    if ( *pParseBuffer == '<' || *( pParseBuffer + 1 ) == '?' ) {
        pParseBuffer += 2;
        while ( *pParseBuffer != '<' ) ++pParseBuffer;
    }

    // parse
    if ( !parse(pParseBuffer, pDoc) ) goto err_buffer;

    free( pBuff );
    close(fd);
    return TRUE;
 
err_buffer:
    free( pBuff );
    close( fd );
    return FALSE;
}

static void showNode( struct tagXMLNode *pNode, int level )
{
    if ( NULL == pNode ) return;
    CHAR *strIndent = (CHAR *)malloc( sizeof(level + 1) );
    memset(strIndent, '\t', level);
    *( strIndent + level ) = '\0';

    printf("%s%s:  %s\n", strIndent, pNode->strName, pNode->strValue);
   
    // print attribute
    if ( pNode->pAttribute ) {
        printf("%sattributes:\n", strIndent);
        struct tagXMLAttribute  *pAtt =  pNode->pAttribute;
        while( pAtt ) {
            printf("%s    %s:  %s\n", strIndent, pAtt->strName, pAtt->strValue);
            pAtt = pAtt->pNext;
        }
    }

    // print children
    if ( pNode->pChild ) showNode( pNode->pChild, level + 1 );

    // print brothers
    if ( pNode->pNext ) showNode( pNode->pNext, level ); 
}

void releaseNode(struct tagXMLNode *pNode)
{
    if ( NULL == pNode ) return;
    // release att
    struct tagXMLAttribute *pCur = pNode->pAttribute;
    struct tagXMLAttribute *pDel;
    while ( pCur ) {
        pDel = pCur;
        pCur = pCur->pNext;
        free( pDel );
    }
    pNode->pAttribute = NULL;

    // release child
    if ( pNode->pChild ) releaseNode( pNode->pChild );

    // release brothers
    if ( pNode->pNext ) releaseNode( pNode->pNext );   
}

void releaseXML( struct tagXMLDocument *pDoc)
{
    if ( NULL == pDoc ) return;

    releaseNode( pDoc->pChild );
    
    pDoc->uiChildSize   = 0;
    pDoc->pChild        = NULL;
}

void showXML( struct tagXMLDocument *pDoc)
{
    if ( NULL == pDoc ) return;

    // just one root node
    if ( pDoc->pChild )
        showNode( pDoc->pChild, 1 );
}

struct tagXMLNode* getNode( const CHAR *strName, struct tagXMLDocument *pDoc )
{
    struct tagXMLNode *pNode = pDoc->pChild;
    while ( NULL != pNode ) {
        if ( strncmp( pNode->strName, strName, 128 ) == 0 ) return pNode;

        pNode = pNode->pNext;
    }

    return NULL;
}

struct tagXMLNode* getChildNode( const CHAR *strName, struct tagXMLNode *pNode )
{
    struct tagXMLNode *pChild = pNode->pChild;
    while ( NULL != pChild ) {
        if ( strncmp( pChild->strName, strName, 128 ) == 0 ) return pChild;

        pChild = pChild->pNext;
    }

    return NULL;
}

struct tagXMLAttribute*  getAttribute( const CHAR *strName, struct tagXMLNode *pNode )
{
    struct tagXMLAttribute *pAtt = pNode->pAttribute;
    while ( NULL != pAtt ) {
        if ( strncmp( pAtt->strName, strName, 128 ) == 0 ) return pAtt;

        pAtt = pAtt->pNext;
    }
    return NULL;
}

// inner function
BOOLEAN	isReturn(CHAR cChar)
{
    return ((cChar == '\r') || (cChar == '\n'));
}

 CHAR* skipWhiteSpace( CHAR* pcData )
 {
    if ( pcData == NULL ) return NULL;
    while (*pcData != L'\0') {
        if ( !( (*pcData == ' ') || (*pcData == '\t') || (*pcData == '\r') || (*pcData == '\n') ) ) {
            return pcData;
        }
        pcData++;
    }
    return NULL;
}

BOOLEAN checkCh(CHAR ch)
{
    if ( ( ch >= '0' && ch <= '9' ) || \
            ( ch >= 'a' && ch <= 'z' ) || \
            ( ch >= 'A' && ch <= 'Z' ) || \
            ( ch == '_' ) || ( ch == '-' ) || ( ch == '.' ) || ( ch == ':' ) ) {
            
            return TRUE;
    }
    return FALSE;
}

static BOOLEAN parseNode( CHAR** pcDataPtr, struct tagXMLNode *pNode )
{
    if ( pcDataPtr == NULL ) return FALSE;

    CHAR *pcData = *pcDataPtr;
    if ( pcData == NULL || pNode == NULL ) return FALSE;
    if ( *pcData != '<' ) return FALSE;
    ++pcData;       // skip '<'
    pcData = skipWhiteSpace( pcData );  // skip whitespace begin

    CHAR *strName       = pNode->strName;
    CHAR *strValue      = pNode->strValue;
    BOOLEAN bParseValue = FALSE;
    BOOLEAN bSeparator  = FALSE;
    
    // parse name and attribute
    while ( *pcData != '>' && *pcData != '\0' ) {
        switch ( *pcData ) {
        case ' ':
            // new attribute
            pcData = skipWhiteSpace( pcData );
            if ( (*pcData == '>') || (*pcData == '\0') ) break;

            struct tagXMLAttribute *pAttribute = (struct tagXMLAttribute *)malloc( sizeof(struct tagXMLAttribute) );
            if ( pAttribute == NULL ) return FALSE;
            pAttribute->pParent  = pNode;
            pAttribute->pNext    = NULL;
            memset( pAttribute->strName, 0, 128 );
            memset( pAttribute->strValue, 0, 64 );

            if ( pNode->pAttribute ) {
                // append to last att
                struct tagXMLAttribute *pAtt = pNode->pAttribute;
                while ( pAtt->pNext != NULL ) pAtt = pAtt->pNext;
                pAtt->pNext = pAttribute;
            } else {
                pNode->pAttribute = pAttribute;
            }
            strName     = pAttribute->strName;
            strValue    = pAttribute->strValue;
            break;
        case '=':
            // attribute value
            bParseValue = TRUE;
            ++pcData;
            break;
        case '\"':
        case '\'':
            bSeparator = ( bSeparator == FALSE );
            // attribute value separator
            ++pcData;   //just skip, no match check
            break; 
        case '/':
            ++pcData;
            pcData = skipWhiteSpace( pcData );
            // skip if in separator
            if ( bSeparator ) break;
           
            // treat as the end of this node
            if ( *pcData != '>' ) return FALSE;

            return TRUE;
        default:
            {
                // parse name or value
                INT i = 0;
                if ( bParseValue ) {
                    while ( checkCh( *pcData ) || *pcData == '/' )
                        strValue[ i++ ] = *( pcData++ ); 
                    bParseValue = FALSE;
                } else {
                    while ( checkCh( *pcData ) )
                        strName[ i++ ] = *( pcData++ );
                }
            }
            break;  
        }
    }

    //printf( "parse name: %s\n", pNode->strName );
    // ======================================
    assert_r( *pcData != '\0', FALSE );
    ++pcData;   // skip '>'
    pcData = skipWhiteSpace( pcData );

    while ( *pcData == '<' && *(pcData + 1) != '/' ) {  // append all child node
        // means a child node
        //printf( "parse child node: %s, parent: %s\n", pcData, pNode->strName );
        struct tagXMLNode *pChildNode = ( struct tagXMLNode * )malloc( sizeof(struct tagXMLNode) );
        assert_r( pChildNode != NULL, FALSE );
        
        pChildNode->pParent      = pNode;
        pChildNode->pRoot        = pNode->pRoot;
        pChildNode->pAttribute   = NULL;
        pChildNode->pChild       = NULL;
        pChildNode->pNext        = NULL;
        memset( pChildNode->strName, 0, 128 );
        memset( pChildNode->strValue, 0, 64 );

        // append to child list
        if ( pNode->pChild ) {
            struct tagXMLNode *pLast = pNode->pChild;
            while ( pLast->pNext != NULL ) pLast = pLast->pNext;
            pLast->pNext = pChildNode;
        } else {
            pNode->pChild = pChildNode;
        }
        assert_r( parseNode( &pcData, pChildNode ), FALSE );
        pcData = skipWhiteSpace( pcData );
    }

    // parse value
    INT i = 0;               
    while ( checkCh( *pcData ) || *pcData == '/' )
        pNode->strValue[ i++ ] = *( pcData++ );
    
    // parse tail
    assert_r( *pcData != '\0', FALSE );
    pcData = skipWhiteSpace( pcData );
    
    assert_r( *pcData == '<' && *( pcData + 1 ) == '/', FALSE ); 
    pcData += 2;

    CHAR strTailName[128];
    memset( strTailName, 0, 128 );
    i = 0; 
    while ( checkCh( *pcData ) )
        strTailName[ i++ ] = *( pcData++ );
    pcData = skipWhiteSpace( pcData );
    //printf( "compare: %s, %s\n", strTailName, pNode->strName );  
    if ( *pcData != '>' || strncmp(strTailName, pNode->strName, 128) != 0 ) return FALSE;

    *pcDataPtr = pcData + 1; // skip '>'
    return TRUE;
}

static BOOLEAN parse( CHAR* pcData, struct tagXMLDocument *pDoc )
{
    if ( pcData == NULL || pDoc == NULL ) return FALSE;

    //skipWhiteSpace
    pcData = skipWhiteSpace( pcData ); 
    if ( (*pcData == '\0') || (*pcData != '<') ) return FALSE;
    struct tagXMLNode *pNode = (struct tagXMLNode *)malloc( sizeof(struct tagXMLNode) );
    if ( pNode == NULL ) return FALSE;
    pDoc->pChild        = pNode;
    pDoc->uiChildSize   = 1;
    
    pNode->pParent      = NULL;
    pNode->pRoot        = pDoc;
    pNode->pAttribute   = NULL;
    pNode->pChild       = NULL;
    pNode->pNext        = NULL;
    memset( pNode->strName, 0, 128 );
    memset( pNode->strValue, 0, 64 );

    return parseNode(&pcData, pNode);
}