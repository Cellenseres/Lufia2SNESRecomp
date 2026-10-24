/* Layout-only preparation. GPU admission and owned frame work are separate. */
enum { MENU_INDEXED_CLAIMS=LUFIA2_MENU_UI_MAX_OBJECTS+MAX_RECORDS,
       MENU_INDEXED_AXES=SNESRECOMP_PPU_JOB_MAX_PRIMITIVES };
typedef struct MenuIndexedBuffer {
    Lufia2MenuUiIndexedClaim claims[MENU_INDEXED_CLAIMS];
    Lufia2MenuUiIndexedObject objects[LUFIA2_MENU_UI_MAX_OBJECTS];
    SnesRecompPpuAxisRun axes[MENU_INDEXED_AXES];
    uint8_t owners[128];
} MenuIndexedBuffer;
typedef struct MenuIndexedStorage {
    MenuIndexedBuffer buffers[2];
    Lufia2MenuUiIndexedLayout view;
    Lufia2MenuUiObject sources[LUFIA2_MENU_UI_MAX_OBJECTS];
    MenuDestination targets[LUFIA2_MENU_UI_MAX_OBJECTS];
    uint64_t layout;
    uint32_t scene;
    unsigned count,active;
    bool valid;
} MenuIndexedStorage;
static MenuIndexedStorage *s_menu_indexed;
bool Lufia2MenuUiReserveIndexedLayout(void) {
    if(!s_menu_indexed)s_menu_indexed=calloc(1,sizeof *s_menu_indexed);
    return s_menu_indexed!=NULL;
}
static uint32_t MenuIndexedAxisBudget(void) {
    return MENU_INDEXED_AXES;
}
static void MenuIndexedLayoutReset(void) {
    if(s_menu_indexed) s_menu_indexed->valid=false;
}
static bool MenuIndexedAxis(MenuIndexedBuffer *b,uint32_t *used,unsigned source,
    unsigned target,unsigned first,unsigned end,bool repeating,uint32_t *offset,uint16_t *length) {
    *offset=*used;
    if(first==end) {*length=0;return true;}
    uint32_t count=0;
    if(repeating) {
        if(snesrecomp_ppu_repeat_axis(source,target,first,end,b->axes+*used,
            MenuIndexedAxisBudget()-*used,&count)!=SNESRECOMP_AXIS_OK || count>UINT16_MAX) return false;
    } else {
        if(*used==MenuIndexedAxisBudget()) return false;
        b->axes[*used]=(SnesRecompPpuAxisRun){(uint16_t)first,(uint16_t)first,
            (uint16_t)(end-first),1,0};count=1;
    }
    *used+=count;*length=(uint16_t)count;return true;
}
const Lufia2MenuUiIndexedLayout *Lufia2MenuUiIndexedLayoutPlan(void) {
    if(!s_active || s_move_highlight || s_preview_path[0] || s_width<WIDTH ||
        s_width>SNESRECOMP_PPU_MAX_WIDTH) return NULL;
    if(!s_menu_indexed) s_menu_indexed=calloc(1,sizeof *s_menu_indexed);
    MenuIndexedStorage *p=s_menu_indexed;if(!p) return NULL;
    for(unsigned i=0;i<s_count;++i) s_targets[i]=ResolveObjectDestination(i,0);
    const bool same=p->valid && p->view.width==s_width && p->scene==s_scene &&
        p->layout==s_layout_generation && p->count==s_count &&
        MenuPrimitiveObjectsEqual(p->sources,s_objects,s_count) &&
        MenuPrimitiveTargetsEqual(p->targets,s_targets,s_count) &&
        snesrecomp_u8_equal(p->buffers[p->active].owners,s_oam_slot,sizeof s_oam_slot);
    if(same) return &p->view;
    /* All output goes to the inactive buffer; refusal leaves the old view
     * intact. A failed candidate must not be presented as the current layout. */
    MenuIndexedBuffer *b=p->buffers+(p->active^1u);
    uint32_t claims=0,objects=0,axes=0;
    const unsigned extra=(s_width-WIDTH)/2u;
    for(unsigned i=0;i<s_count;++i) {
        const Lufia2MenuUiObject *o=s_objects+i;
        if(o->kind==GROUP || o->kind==PARTY_HIGHLIGHT) continue;
        const unsigned plane=o->kind==WINDOW?0u:o->kind==TEXT?1u:2u;
        const unsigned owner=plane==2?o->slot:i;
        b->claims[claims++]=(Lufia2MenuUiIndexedClaim){(uint16_t)(o->x+extra),(uint16_t)o->y,
            o->width,o->height,(uint16_t)owner,(uint8_t)plane,1};
        const MenuDestination *d=s_targets+i;
        if(!d->visible || d->width<=0 || d->height<=0) continue;
        const int dw=o->kind==WINDOW || d->width<o->width?d->width:o->width;
        const int dh=o->kind==WINDOW || d->height<o->height?d->height:o->height;
        const int left=d->x<0?-d->x:0,top=d->y<0?-d->y:0;
        const int right=dw<(int)s_width-d->x?dw:(int)s_width-d->x;
        const int bottom=dh<HEIGHT-d->y?dh:HEIGHT-d->y;
        if(left>=right || top>=bottom) continue;
        Lufia2MenuUiIndexedObject next={.x=d->x,.y=d->y,
            .source_x=(uint16_t)(o->x+extra),.source_y=(uint16_t)o->y,
            .owner=(uint16_t)owner,.plane=(uint8_t)plane};
        if(!MenuIndexedAxis(b,&axes,o->width,(unsigned)d->width,(unsigned)left,(unsigned)right,
                o->kind==WINDOW,&next.x_first,&next.x_count) ||
           !MenuIndexedAxis(b,&axes,o->height,(unsigned)d->height,(unsigned)top,(unsigned)bottom,
                o->kind==WINDOW,&next.y_first,&next.y_count)) return NULL;
        b->objects[objects++]=next;
    }
    /* Region overrides affect text ownership, not native coverage. Keep their
     * exact original order after ordinary object claims, including hidden UI. */
    for(unsigned r=0;r<s_layout_count;++r) {
        const MenuLayoutRecord *l=s_layout+r;
        if(l->scene!=s_scene || l->kind!=TEXT_REGION) continue;
        const int i=FindObject(l->id);if(i<0) continue;
        const Lufia2MenuUiObject *o=s_objects+i;
        if(claims==MENU_INDEXED_CLAIMS) return NULL;
        b->claims[claims++]=(Lufia2MenuUiIndexedClaim){(uint16_t)(o->x+extra),(uint16_t)o->y,
            o->width,o->height,(uint16_t)i,1,0};
    }
    if(p->view.generation==UINT64_MAX) return NULL;
    memcpy(b->owners,s_oam_slot,sizeof s_oam_slot);
    memcpy(p->sources,s_objects,s_count*sizeof *s_objects);
    memcpy(p->targets,s_targets,s_count*sizeof *s_targets);
    p->view=(Lufia2MenuUiIndexedLayout){b->claims,b->objects,b->axes,b->owners,
        claims,objects,axes,p->view.generation+1u,(uint16_t)s_width,HEIGHT};
    p->layout=s_layout_generation;p->scene=s_scene;p->count=s_count;
    p->active^=1u;p->valid=true;
    return &p->view;
}
SnesRecompPpuResult Lufia2MenuUiFreezeIndexedLayout(SnesRecompPpuJob *job,
    const SnesRecompPpuFrame *base,const SnesRecompPpuFrame *graphics,
    const SnesRecompPpuObjRun *runs,uint32_t run_count,const uint32_t qualified_lines[8],uint64_t sequence) {
    const Lufia2MenuUiIndexedLayout *p=Lufia2MenuUiIndexedLayoutPlan();
    if(!p) return SNESRECOMP_PPU_UNSUPPORTED;
    if(!qualified_lines) return SNESRECOMP_PPU_INCOMPLETE_MEMORY;
    const SnesPpuFrameCapture *capture=snesrecomp_ppu_frame_capture(graphics);
    if(!capture) return SNESRECOMP_PPU_INVALID;
    uint16_t band_materials[SNES_PPU_MAX_BANDS*3];
    for(unsigned band=0;band<capture->band_count;++band)
        for(unsigned plane=0;plane<3;++plane)
            band_materials[band*3u+plane]=(uint16_t)MenuPrimitiveMaterial(capture->bands+band,plane);
    SnesRecompPpuIndexedLayout layout={.claims=p->claims,.objects=p->objects,
        .axes=p->axes,.oam_owners=p->oam_owners,.claim_count=p->claim_count,
        .object_count=p->object_count,.axis_count=p->axis_count,
        .source_layers={0,2,4},.plane_count=3,.owner_planes=6,.width=p->width,.height=p->height,
        .obj_runs=runs,.obj_run_count=run_count,.materials=s_menu_materials,.material_count=6,
        .band_materials=band_materials,.band_count=(uint16_t)capture->band_count};
    memcpy(layout.qualified_lines,qualified_lines,sizeof layout.qualified_lines);
    return snesrecomp_ppu_job_freeze_indexed(job,base,graphics,&layout,sequence);
}
