// Менеджер быстрого распределения памяти для простых объектов

#pragma once

#include <stdlib.h>

const int MAX_INT = 0x7FFFFFFF;

template <class T> struct FMBlock
{
   union
   {
      FMBlock<T>   *next;
      T             data;
   };
};

template <class T> struct FMPage
{
   FMPage<T>*         next;
   FMBlock<T>*        blocks;
   int                size;

   FMPage( int page_size ): size(page_size), next(0)
   {
      blocks = new FMBlock<T>[size];
   };

   ~FMPage()
   {
     delete[] blocks;
     if( next )
       delete next;
   }
};

template <class T> class FastMemManager
{
   public:
         FastMemManager( int page_size, double page_size_factor ):
           firstPage(0),
           freeBlocks(0),
           pageSize( page_size > 64 ? page_size : 64 ),
           pageSizeFactor(page_size_factor) {}

         ~FastMemManager()
         {
           if( firstPage )
             delete firstPage;
         }

      T* Allocate()
      {
         if( freeBlocks == NULL )
           AddPage();

         FMBlock<T> *b = freeBlocks;
         freeBlocks = freeBlocks->next;
         return &(b->data);
      }

      void Free( T* t )
      {
         FMBlock<T> *b = (FMBlock<T>*)t;
         b->next  = freeBlocks;
         freeBlocks = b;
      }

      void FreeAll()
      {
         freeBlocks = NULL;
         FMPage<T> *page = firstPage;
         while( page )
         {  
            for( int i = 0; i<page->size; i++ )
            {
               page->blocks[i].next = freeBlocks;
               freeBlocks = &(firstPage->blocks[i]);
            }
            page = page->next;
         }
      }

      void SetPageSize( int size ) { pageSize = size > 8 ? size : 8; }

   private:
            FMPage<T>*         firstPage;
            FMBlock<T>        *freeBlocks;
            int                pageSize;
            double             pageSizeFactor;

      void AddPage()
      {
         FMPage<T> *page = new FMPage<T>( pageSize );

         page->next = firstPage;
         firstPage = page;

         for( int i = 0; i < pageSize; ++i )
         {
            firstPage->blocks[i].next = freeBlocks;
            freeBlocks = &(firstPage->blocks[i]);
         }

         if( pageSize < MAX_INT / pageSizeFactor )
            pageSize = (int)(pageSizeFactor*pageSize);
      }
};
