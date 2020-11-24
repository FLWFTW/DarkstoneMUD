#include "mud.h"
#include <stdio.h>

char *escape_string( char *s )
{
   int i = 0, j=0;
   static char ret[MAX_STRING_LENGTH*2];
   
   for( i = 0; i < strlen( s ); i++ )
   {
      if( j == MAX_STRING_LENGTH ) break; //we've run out of room in the return buffer
      switch( s[i] )
      {
         default:
         {
            ret[j] = s[i];
            j++;
            break;
         }
         case '\n':
         {
            ret[j] = '\\'; //escaped '\'
            j++;
            ret[j] = 'n'; //the n in the escape string;
            j++;
            break;
         }
         case '\r':
         {
            ret[j] = '\\'; //escaped '\'
            j++;
            ret[j] = 'r'; //the r in the escape string;
            j++;
            break;
         }
         case '\'':
         {
            ret[j] = '\\'; //escaped '\'
            j++;
            ret[j] = '\''; //the ' in the escape string;
            j++;
            break;
         }
         case '\"':
         {
            ret[j] = '\\'; //escaped '\'
            j++;
            ret[j] = '\"'; //the " in the escape string;
            j++; //so it should look like \" now, instead of just a "
            break;
         }
         case'\0':
         {
            ret[j] = '\0';
            return ret;
         }
      }
   }
   j++;
   ret[j] = '\0';
   return ret;
}

void save_reset_data( FILE *fp, RESET_DATA *r )
{
   RESET_DATA *p;
   if( !fp || !r )
      return;
      
   fprintf( fp, "resets={\n" );
   
   for( p = r; p; p = p->next )
   {
      
      fprintf( fp, "   {\n" );
      fprintf( fp, "      command = '%c';\n", p->command );
      fprintf( fp, "      extra = %i;\n", p->extra );
      fprintf( fp, "      arg1 = %i;\n", p->arg1 );
      fprintf( fp, "      arg2 = %i;\n", p->arg2 );
      fprintf( fp, "      arg3 = %i;\n", p->arg3 );
      fprintf( fp, "   },\n" );
   
   }
   
   fprintf( fp, "}\n" );
   
   return;
}

void save_room_data( FILE *fp, AREA_DATA *a )
{
   int i;
   ROOM_INDEX_DATA *r;
   MPROG_DATA *mprog;
   ExitData *xit;
   ExtraDescData *ed;
   
   if( !fp || !a )
      return;
   
   fprintf( fp, "rooms={\n" );
   for( i = a->low_r_vnum; i <= a->hi_r_vnum; i++ )
   {
      if( ( r = get_room_index( i ) ) == NULL )
         continue;
      
      fprintf( fp, "   {\n" );
      fprintf( fp, "      vnum=%i;\n", i );
      fprintf( fp, "      name=\"%s\";\n", escape_string( r->name_.c_str() ) );
      fprintf( fp, "      description=\"%s\";\n", escape_string( r->description_.c_str() ) );
      fprintf( fp, "      random_room_type=%i;\n", r->random_room_type );
      fprintf( fp, "      random_description=%i;\n", r->random_description );
      fprintf( fp, "      room_flags=%i;\n", r->room_flags );
      fprintf( fp, "      sector_type=%i;\n", r->sector_type );
      fprintf( fp, "      tele_delay=%i;\n", r->tele_delay );
      fprintf( fp, "      tele_vnum=%i;\n", r->tele_vnum );
      
      //exit data
      fprintf( fp, "      exit_data=\n      {\n" );
      for( xit = r->first_exit; xit; xit=xit->next )
      {
         if( IS_SET( xit->exit_info, EX_PORTAL ) )
            continue; //don't fold portals
         fprintf( fp, "         {\n" );
         fprintf( fp, "            vdir=%i;\n", xit->vdir );
         fprintf( fp, "            description=\"%s\";\n", escape_string( xit->description_.c_str() ) );
         fprintf( fp, "            keyword=\"%s\";\n", escape_string( xit->keyword_.c_str() ) );
         fprintf( fp, "            exit_info=%i;\n", xit->exit_info & ~EX_BASHED );
         fprintf( fp, "            key=%i;\n", xit->key );
         fprintf( fp, "            vnum=%i;\n", xit->vnum );
         fprintf( fp, "            distance=%i;\n", xit->distance );
         fprintf( fp, "         },\n" );
      }
      fprintf( fp, "      }\n" );
      //end exit data
      
      //extra descriptions
      fprintf( fp, "      extra_descs=\n      {\n" );
      for( ed = r->first_extradesc; ed; ed=ed->next )
      {
         fprintf( fp, "         {\n" );
         fprintf( fp, "            keywords=\"%s\";\n", escape_string( ed->keyword_.c_str() ) );
         fprintf( fp, "            description=\"%s\";\n", escape_string( strip_cr( ed->description_.c_str() ) ) );
         fprintf( fp, "         },\n" );
      }
      fprintf( fp, "      }\n" );
      //end extra descriptions
      
      //start room progs
      fprintf( fp, "      room_progs=\n      {\n" );
      for( mprog = r->mudprogs; mprog; mprog = mprog->next )
      {
         fprintf( fp, "         {\n" );
         if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
         {
            if( mprog->type == IN_FILE_PROG )
            {
               fprintf( fp, "            type=\"%s\";\n", mprog_type_to_name( mprog->type ) );
               fprintf( fp, "            arglist=\"%s\";\n", mprog->arglist );
            }
            /*
             * Don't let it save progs which came from files. That would be silly. 
             */
            else if( mprog->comlist && mprog->comlist[0] != '\0' )
            {
               fprintf( fp, "            type=\"%s\";\n", mprog_type_to_name( mprog->type ) );
               fprintf( fp, "            arglist=\"%s\";\n", mprog->arglist );
               fprintf( fp, "            comlist=\"%s\";\n", escape_string( strip_cr( mprog->comlist ) ) );
            }
         }
         fprintf( fp, "         },\n" );
      }
      fprintf( fp, "      }\n" );
      //end room progs
      
      fprintf( fp, "   },\n" );
      //end single room
   }
   fprintf( fp, "}\n" );
}

void save_mobile_data( FILE *fp, AREA_DATA *a )
{
   int i;
   MOB_INDEX_DATA *m;
   MPROG_DATA *mprog;
   
   if( !fp || !a )
      return;
   
   fprintf( fp, "\nmobiles={\n" ); //start mobile section
   
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL )
         continue;
      
      //Old SMAUG has complex mobs and non-complex mobs, I believe to save space
      //on area files. We will just assume all mobs are complex and save all
      //mob data.
      fprintf( fp, "   {\n" ); //start mobile
      fprintf( fp, "      vnum=%i;\n", i );
      fprintf( fp, "      player_name=\"%s\";\n", escape_string( m->playerName_.c_str() ) );
      fprintf( fp, "      short_descr=\"%s\";\n", escape_string( m->shortDesc_.c_str() ) );
      fprintf( fp, "      long_descr=\"%s\";\n", escape_string( strip_cr( m->longDesc_.c_str() ) ) );
      fprintf( fp, "      description=\"%s\";\n", escape_string( strip_cr( m->description_.c_str() ) ) );
      fprintf( fp, "      act=%i;\n", m->act  );
      fprintf( fp, "      affected_by=%i;\n", m->affected_by  );
      fprintf( fp, "      level = %i;\n", m->level );
      fprintf( fp, "      mobthac0 = %i;\n", m->mobthac0 );
      fprintf( fp, "      ac=%i\n", m->ac );
      fprintf( fp, "      damnodice=%i;damsizedice=%i;damplus=%i;\n", m->damnodice, m->damsizedice, m->damplus );
      fprintf( fp, "      hitnodice=%i;hitsizedice=%i;hitplus=%i;\n", m->hitnodice, m->hitsizedice, m->hitplus );
      fprintf( fp, "      gold=%i;\n", m->gold );
      fprintf( fp, "      experience=%i;\n", m->exp );
      fprintf( fp, "      position=%i;\n", m->position+100 );
      fprintf( fp, "      defposition=%i;\n", m->defposition+100 );
      fprintf( fp, "      perm_str=%i;\n", m->perm_str );
      fprintf( fp, "      perm_int=%i;\n", m->perm_int );
      fprintf( fp, "      perm_wis=%i;\n", m->perm_wis );
      fprintf( fp, "      perm_dex=%i;\n", m->perm_dex );
      fprintf( fp, "      perm_con=%i;\n", m->perm_con );
      fprintf( fp, "      perm_cha=%i;\n", m->perm_cha );
      fprintf( fp, "      perm_lck=%i;\n", m->perm_lck );
      fprintf( fp, "      saving_poison_death=%i;\n", m->saving_poison_death );
      fprintf( fp, "      saving_wand=%i;\n", m->saving_wand );
      fprintf( fp, "      saving_para_petri=%i;\n", m->saving_para_petri );
      fprintf( fp, "      saving_breath=%i;\n", m->saving_breath );
      fprintf( fp, "      saving_spell_staff=%i;\n", m->saving_spell_staff );
      fprintf( fp, "      race=%i;\n", m->race );
      fprintf( fp, "      class=%i;\n", m->Class );
      fprintf( fp, "      height=%i;\n", m->height );
      fprintf( fp, "      weight=%i;\n", m->weight );
      fprintf( fp, "      speaks=%i;\n", m->speaks );
      fprintf( fp, "      speaking=%i;\n", m->speaking );
      fprintf( fp, "      numattacks=%i;\n", m->numattacks );
      fprintf( fp, "      hitroll=%i;\n", m->hitroll );
      fprintf( fp, "      damroll=%i;\n", m->damroll );
      fprintf( fp, "      xflags=%i;\n", m->xflags );
      fprintf( fp, "      resistant=%i;\n", m->resistant );
      fprintf( fp, "      immune=%i;\n", m->immune );
      fprintf( fp, "      susceptible=%i;\n", m->susceptible );
      fprintf( fp, "      attacks=%i;\n", m->attacks );
      fprintf( fp, "      defenses=%i;\n", m->defenses  );
      if( m->mudprogs )
      {
         fprintf( fp, "      mprogs=\n      {\n" ); //start prog section
         for( mprog = m->mudprogs; mprog; mprog=mprog->next )
         {
            fprintf( fp, "         {\n" ); //start mob prog
            if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
            {
               if( mprog->type == IN_FILE_PROG )
               {
                  fprintf( fp, "            type=\"%s\";\n", mprog_type_to_name( mprog->type ) );
                  fprintf( fp, "            arglist=\"%s\";\n", mprog->arglist );
               }
               /*
                * Don't let it save progs which came from files. That would be silly. 
                */
               else if( mprog->comlist && mprog->comlist[0] != '\0' )
               {
                  fprintf( fp, "            type=\"%s\";\n", mprog_type_to_name( mprog->type ) );
                  fprintf( fp, "            arglist=\"%s\";\n", mprog->arglist );
                  fprintf( fp, "            comlist=\"%s\";\n", strip_cr( mprog->comlist ) );
               }
            }
            fprintf( fp, "         },\n" ); //end mob prog
         }
         fprintf( fp, "      },\n" ); //end prog section
      }
      fprintf( fp, "   },\n" ); //end mobile
      
   }
   
   fprintf( fp, "}\n" ); //end mobile section
   
   return;
}

void save_object_data( FILE *fp, AREA_DATA *a )
{
   int i, val0, val1, val2, val3, val4, val5;
   OBJ_INDEX_DATA *o;
   MPROG_DATA *mprog;
   ExtraDescData *ed;
   AFFECT_DATA *af;
   
   if( !fp || !a )
      return;
   
   fprintf( fp, "\nobjects={\n" );
   for( i = a->low_o_vnum; i <= a->hi_o_vnum; i++ )
   {
      if( ( o = get_obj_index( i ) ) == NULL )
         continue;
      
      fprintf( fp, "   {\n" );
      fprintf( fp, "      vnum=%i;\n", i );
      fprintf( fp, "      name=\"%s\";\n", escape_string( o->name_.c_str() ) );
      fprintf( fp, "      short_desc=\"%s\";\n", escape_string( o->shortDesc_.c_str() ) );
      fprintf( fp, "      action_desc=\"%s\";\n", escape_string( o->actionDesc_.c_str() ) );
      fprintf( fp, "      long_desc=\"%s\";\n", escape_string( o->longDesc_.c_str() ) );
      fprintf( fp, "      item_type=%i;\n", o->item_type );
      fprintf( fp, "      extra_flags=%i;\n", o->extra_flags );
      fprintf( fp, "      wear_flags=%i;\n", o->wear_flags );
      fprintf( fp, "      layers=%i;\n", o->layers );
      fprintf( fp, "      level=%i;\n", o->level );
      
      
      val0 = o->value[0];
      val1 = o->value[1];
      val2 = o->value[2];
      val3 = o->value[3];
      val4 = o->value[4];
      val5 = o->value[5];
      switch ( o->item_type )
      {
         case ITEM_PILL:
         case ITEM_POTION:
         case ITEM_SCROLL:
         {
            if ( IS_VALID_SN(val1) ) val1 = skill_table[val1]->slot;
            if ( IS_VALID_SN(val2) ) val2 = skill_table[val2]->slot;
            if ( IS_VALID_SN(val3) ) val3 = skill_table[val3]->slot;
            break;
         }
         case ITEM_STAFF:
         case ITEM_WAND:
         {
            if ( IS_VALID_SN(val3) ) val3 = skill_table[val3]->slot;
            break;
         }
         case ITEM_SALVE:
         {
            if ( IS_VALID_SN(val4) ) val4 = skill_table[val4]->slot;
            if ( IS_VALID_SN(val5) ) val5 = skill_table[val5]->slot;
            break;
         }
      }
      fprintf( fp, "      val0=%i;\n", val0 );
      fprintf( fp, "      val1=%i;\n", val1 );
      fprintf( fp, "      val2=%i;\n", val2 );
      fprintf( fp, "      val3=%i;\n", val3 );
      fprintf( fp, "      val4=%i;\n", val4 );
      fprintf( fp, "      val5=%i;\n", val5 );
      fprintf( fp, "      weight=%i;\n", o->weight );
      fprintf( fp, "      cost=%i;\n", o->cost );
      fprintf( fp, "      rent=%i;\n", o->rent ? o->rent : (int)o->cost/1 );
      fprintf( fp, "      rare=%i;\n", o->rare );
      
      //extra descriptions
      fprintf( fp, "      extra_descs=\n      {\n" );
      for( ed = o->first_extradesc; ed; ed=ed->next )
      {
         fprintf( fp, "         {\n" );
         fprintf( fp, "            keywords=\"%s\";\n", ed->keyword_.c_str() );
         fprintf( fp, "            description=\"%s\";\n", escape_string( strip_cr( ed->description_.c_str() ) ) );
         fprintf( fp, "         },\n" );
      }
      fprintf( fp, "      }\n" );
      //end extra descriptions
      
      //affect data
      fprintf( fp, "      affect_data=\n      {\n" );
      for( af = o->first_affect; af; af=af->next )
      {
         fprintf( fp, "         {\n" );
         fprintf( fp, "            location=%i;\n", af->location );
         fprintf( fp, "            modifier=%i;\n", ((af->location == APPLY_WEAPONSPELL
                                                       || af->location == APPLY_WEARSPELL
                                                       || af->location == APPLY_REMOVESPELL
                                                       || af->location == APPLY_STRIPSN)
                                                       && IS_VALID_SN(af->modifier))
                                                       ? skill_table[af->modifier]->slot : af->modifier );
         fprintf( fp, "         },\n" );
      }
      fprintf( fp, "      }\n" );
      //end affect data
      
      //obj progs
      if( o->mudprogs )
      {
         fprintf( fp, "      obj_progs=\n      {\n" );
         for( mprog=o->mudprogs; mprog; mprog=mprog->next )
         {
            fprintf( fp, "         {\n" );
            fprintf( fp, "            type=\"%s\";\n", mprog_type_to_name( mprog->type ) );
            fprintf( fp, "            arglist=\"%s\";\n", mprog->arglist );
            fprintf( fp, "            comlist=\"%s\";\n", escape_string( strip_cr( mprog->comlist ) ) );
            fprintf( fp, "         },\n" );
         }
      //end obj progs
      fprintf( fp, "      },\n" );
      
      }
      //end object
      fprintf( fp, "   },\n" );
   }
   fprintf( fp, "}\n" );
}

void save_area_data( FILE *fp, AREA_DATA *a )
{
   fprintf( fp, "name = \"%s\";\n", a->name );
   fprintf( fp, "filename = \"%s\";\n", a->filename );
   fprintf( fp, "version = %i;\n", a->version );
   fprintf( fp, "author = \"%s\";\n", a->author );
   fprintf( fp, "install_data = %li;\n", a->secInstallDate );
   fprintf( fp, "resetmsg = \"%s\";\n", a->resetmsg );
   fprintf( fp, "flags = %i;\n", a->flags );
   fprintf( fp, "status = %i;\n", (int)a->status );
   fprintf( fp, "age = %i;\n", (int)a->age );
   fprintf( fp, "nplayer = %i;\n", (int)a->nplayer );
   fprintf( fp, "reset_frequency = %i;\n", (int)a->reset_frequency );
   fprintf( fp, "low_r_vnum = %i;\n", a->low_r_vnum );
   fprintf( fp, "hi_r_vnum = %i;\n", a->hi_r_vnum );
   fprintf( fp, "low_o_vnum = %i;\n", a->low_o_vnum );
   fprintf( fp, "hi_o_vnum = %i;\n", a->hi_o_vnum );
   fprintf( fp, "low_soft_range = %i;\n", a->low_soft_range );
   fprintf( fp, "hi_soft_range = %i;\n", a->hi_soft_range );
   fprintf( fp, "low_hard_range = %i;\n", a->low_hard_range );
   fprintf( fp, "hi_hard_range = %i;\n", a->hi_hard_range );
   fprintf( fp, "max_players = %i;\n", (int)a->max_players );
   fprintf( fp, "mkills = %i;\n", a->mkills );
   fprintf( fp, "mdeaths = %i;\n", a->mdeaths );
   fprintf( fp, "pkills = %i;\n", a->pkills );
   fprintf( fp, "pdeaths = %i;\n", a->pdeaths );
   fprintf( fp, "gold_looted = %i;\n", a->gold_looted );
   fprintf( fp, "illegal_pk = %i;\n", a->illegal_pk );
   fprintf( fp, "high_economy = %i;\n", a->high_economy );
   fprintf( fp, "low_economy = %i;\n\n", a->low_economy );
}

void save_shop_data( FILE *fp, AREA_DATA *a )
{
   SHOP_DATA *s;
   MOB_INDEX_DATA *m;
   int i;
   
   if( !fp || !a )
      return;
      
   fprintf( fp, "\nshops={\n" );
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL ) //if there is no mob with vnum i
         continue;
      if( ( s = m->pShop ) == NULL ) //if the mob is not a shop
         continue;
   
      fprintf( fp, "   {\n" );
      fprintf( fp, "      keeper=%i;\n", s->keeper );
      fprintf( fp, "      buy_type0=%i;\n", s->buy_type[0] );
      fprintf( fp, "      buy_type1=%i;\n", s->buy_type[1] );
      fprintf( fp, "      buy_type2=%i;\n", s->buy_type[2] );
      fprintf( fp, "      buy_type3=%i;\n", s->buy_type[3] );
      fprintf( fp, "      buy_type4=%i;\n", s->buy_type[4] );
      fprintf( fp, "      profit_buy=%i;\n", s->profit_buy );
      fprintf( fp, "      open_hour=%i;\n", s->open_hour );
      fprintf( fp, "      close_hour=%i;\n", s->close_hour );
      fprintf( fp, "      short_desc=\"%s\";\n", m->shortDesc_.c_str() );
      fprintf( fp, "   },\n" );
   }
   
   
   fprintf( fp, "}\n" );
   
   return;
}

void save_stable_data( FILE *fp, AREA_DATA *a )
{
   STABLE_DATA *s;
   MOB_INDEX_DATA *m;
   int i;
   
   if( !fp || !a )
      return;
      
   fprintf( fp, "\nstables={\n" );
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL ) //if there is no mob with vnum i
         continue;
      if( ( s = m->pStable ) == NULL ) //if the mob is not a stable
         continue;
   
      fprintf( fp, "   {\n" );
      fprintf( fp, "      keeper=%i;\n", s->keeper );
      fprintf( fp, "      stable_cost=%i;\n", s->stable_cost );
      fprintf( fp, "      unstable_cost=%i;\n", s->unstable_cost );
      fprintf( fp, "      open_hour=%i;\n", s->open_hour );
      fprintf( fp, "      close_hour=%i;\n", s->close_hour );
      fprintf( fp, "      short_desc=\"%s\";\n", m->shortDesc_.c_str() );
      fprintf( fp, "   },\n" );
   }
   
   
   fprintf( fp, "}\n" );
   
   return;
}

void save_repair_data( FILE *fp, AREA_DATA *a )
{
   REPAIR_DATA *r;
   MOB_INDEX_DATA *m;
   int i;
   
   if( !fp || !a )
      return;
      
   fprintf( fp, "\nrepairs={\n" );
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL ) //if there is no mob with vnum i
         continue;
      if( ( r = m->rShop ) == NULL ) //if the mob is not a stable
         continue;
   
      fprintf( fp, "   {\n" );
      fprintf( fp, "      keeper=%i;\n", r->keeper );
      fprintf( fp, "      fix_type0=%i;\n", r->fix_type[0] );
      fprintf( fp, "      fix_type1=%i;\n", r->fix_type[1] );
      fprintf( fp, "      fix_type2=%i;\n", r->fix_type[2] );
      fprintf( fp, "      profit_fix=%i;\n", r->profit_fix );
      fprintf( fp, "      open_hour=%i;\n", r->open_hour );
      fprintf( fp, "      close_hour=%i;\n", r->close_hour );
      fprintf( fp, "      short_desc=\"%s\";\n", m->shortDesc_.c_str() );
      fprintf( fp, "      shop_type=%i;\n", r->shop_type );
      fprintf( fp, "   },\n" );
   }
   
   
   fprintf( fp, "}\n" );
   
   return;
}

void save_special_data( FILE *fp, AREA_DATA *a )
{
   MOB_INDEX_DATA *m;
   int i;
   
   if( !fp || !a )
      return;
      
   fprintf( fp, "\nspecials={\n" );
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL ) //if there is no mob with vnum i
         continue;
      if( !m->spec_fun ) //if the mob has no special functions
         continue;
   
      fprintf( fp, "   {\n" );
      fprintf( fp, "      vnum=%i;\n", m->vnum );
      fprintf( fp, "      spec_fun=%s", lookup_spec( m->spec_fun ) );
      fprintf( fp, "   },\n" );
   }
   
   
   fprintf( fp, "}\n" );
   
   return;
}

void save_trainer_data( FILE *fp, AREA_DATA *a )
{
   MOB_INDEX_DATA *m;
   int i;
   
   if( !fp || !a )
      return;
      
   fprintf( fp, "\ntrainers={\n" );
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL ) //if there is no mob with vnum i
         continue;
      if( !m->train ) //if the mob is not a trainer
         continue;
   
      fprintf( fp, "   {\n" );
      fprintf( fp, "      vnum=%i;\n", m->vnum );
      fprintf( fp, "      class=%i;\n", m->train->_class );
      fprintf( fp, "      alignment=%i;\n", m->train->alignment );
      fprintf( fp, "      min_level=%i;\n", m->train->min_level );
      fprintf( fp, "      max_level=%i;\n", m->train->max_level );
      fprintf( fp, "      base_cost=%i;\n", m->train->base_cost );
      fprintf( fp, "   },\n" );
   }
   
   
   fprintf( fp, "}\n" );
   
   return;
}

void save_trainer_skills( FILE *fp, AREA_DATA *a )
{
   int i;
   TRAIN_LIST_DATA *t;
   MOB_INDEX_DATA *m;
   SkillType *s;
   
   fprintf( fp, "trainer_skills={\n" );
   for( i = a->low_m_vnum; i <= a->hi_m_vnum; i++ )
   {
      if( ( m = get_mob_index( i ) ) == NULL )
         continue;
      if( !m->train )
         continue;
      for( t = m->train->first_in_list; t; t = t->next )
      {
         if( ( s = get_skilltype( t->sn ) ) == NULL )
            continue;
         fprintf( fp, "   {\n" );
         fprintf( fp, "      vnum=%i;\n", m->vnum );
         fprintf( fp, "      max=%i;\n", t->max );
         fprintf( fp, "      cost=%i;\n", t->cost );
         fprintf( fp, "      skill=\"%s\";\n", s->name_.c_str() );
         fprintf( fp, "   },\n" );
      }
   }
   fprintf( fp, "}" );

   return;
}

void convert_all_areas( void )
{
   AREA_DATA *a;
   FILE *fp;
   char filename[MAX_STRING_LENGTH];
   if( !first_area )
      return;
   
   
   //loop through each area, open the appropriate file, serialize the content
   //to Lua, wash, rinse, repeat.
   for( a = first_area; a; a = a->next )
   {
      //knock off the .are and replace with .laf...
      //(.laf = Lua Area File)
      a->filename[strlen(a->filename)-3] = 'l';
      a->filename[strlen(a->filename)-2] = 'a';
      a->filename[strlen(a->filename)-1] = 'f';
      //pretty gnarly way of doing it, I know
      
      snprintf( filename, MAX_STRING_LENGTH, "./new_format/%s", a->filename ); 
      if( (fp = fopen( filename, "w" ) ) == NULL )
      {
         bug( "Unable to allocate memory for a file! Aborting!" );
         exit(1);
      }
      
      save_area_data( fp, a );
      save_reset_data( fp, a->first_reset );
      save_mobile_data( fp, a );
      save_object_data( fp, a );
      save_room_data( fp, a );
      save_shop_data( fp, a );
      save_stable_data( fp, a );
      save_repair_data( fp, a );
      save_special_data( fp, a );
      save_trainer_data( fp, a );
      save_trainer_skills( fp, a );
      fclose( fp );
   }
   
   return;
}
