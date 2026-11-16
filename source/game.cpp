#include "game.h"
#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <stdio.h>
#include <string.h>
static const char* NAMES[26]={"NEO","ZICO","LUNA","RAIO","KIRA","TITA","BRUNO","PAULO","ANA","MIKO","ZEUS","NOVA","LOBO","FENIX","DUDU","MAGO","THOR","RUBI","SOL","VEGA","KAIO","ICARO","BIA","DANTE","YUKI","CHICO"};
Game::Game(){screen=NULL;running=false;sdlUp=false;gameState=0;cameraX=0;cameraY=0;zoom=1;frameCount=0;rankCount=0;playerRank=1;bestScore=0;deathScore=0;deathLength=0;deathKills=0;deathRank=1; for(int i=0;i<MAX_FOOD;i++)foods[i].active=false; for(int i=0;i<MAX_PARTICLES;i++)particles[i].active=false;}
Game::~Game(){shutdown();}
bool Game::init(){
if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK)<0) return false;
sdlUp=true;
screen=SDL_SetVideoMode(SCREEN_W,SCREEN_H,32,SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
if(!screen) screen=SDL_SetVideoMode(SCREEN_W,SCREEN_H,32,SDL_SWSURFACE|SDL_FULLSCREEN);
if(!screen) return false;
SDL_ShowCursor(SDL_DISABLE);
ioPadInit(7);
srand(12345);
initWorld(); running=true; return true;
}
void Game::shutdown(){ if(!sdlUp) return; sdlUp=false; running=false; ioPadEnd(); SDL_Quit(); screen=NULL; }
void Game::initWorld(){
for(int i=0;i<MAX_SNAKES;i++){snakes[i].alive=false; snakes[i].path.clear(); snakes[i].body.clear();}
spawnSnake(snakes[0],true,0,0);
for(int i=1;i<MAX_SNAKES;i++){float a=((float)rand()/RAND_MAX)*6.283f; float d=500+((float)rand()/RAND_MAX)*3000; spawnSnake(snakes[i],false,cosf(a)*d,sinf(a)*d);}
for(int i=0;i<MAX_FOOD;i++) spawnFood(i);
cameraX=snakes[0].body[0].x; cameraY=snakes[0].body[0].y; updateRanking(); gameState=0;
}
void Game::resetPlayer(){initWorld(); gameState=1;}
void Game::spawnSnake(Snake &s,bool isPlayer,float x,float y){
s.isPlayer=isPlayer; s.alive=true; s.angle=((float)rand()/RAND_MAX)*6.283f; s.targetAngle=s.angle; s.baseSpeed=2.8f; s.boostSpeed=5.8f; s.speed=s.baseSpeed; s.boosting=false; s.boostTimer=0; s.turnSpeed=0.08f; s.aiTimer=0; s.aiTarget=Vec2(x+200,y); s.score=0; s.kills=0;
if(isPlayer){s.length=25; s.radius=14; s.cr=255; s.cg=120; s.cb=180; s.cr2=255; s.cg2=80; s.cb2=150; s.name="VOCE";}
else{s.length=12+rand()%60; s.radius=10+(rand()%8); int c=rand()%6;
if(c==0){s.cr=0;s.cg=255;s.cb=140;s.cr2=0;s.cg2=200;s.cb2=110;}
else if(c==1){s.cr=255;s.cg=50;s.cb=50;s.cr2=200;s.cg2=20;s.cb2=20;}
else if(c==2){s.cr=50;s.cg=150;s.cb=255;s.cr2=20;s.cg2=100;s.cb2=220;}
else if(c==3){s.cr=255;s.cg=200;s.cb=0;s.cr2=220;s.cg2=150;s.cb2=0;}
else if(c==4){s.cr=200;s.cg=50;s.cb=255;s.cr2=150;s.cg2=20;s.cb2=220;}
else{s.cr=0;s.cg=220;s.cb=220;s.cr2=0;s.cg2=170;s.cb2=170;}
s.name=NAMES[rand()%26];}
s.path.clear(); s.body.clear();
for(int i=0;i<(int)s.length*2;i++) s.path.push_back(Vec2(x-cosf(s.angle)*i*8,y-sinf(s.angle)*i*8));
for(int i=0;i<(int)s.length;i++) s.body.push_back(s.path[i*2]);
}
void Game::spawnFood(int idx){
float a=((float)rand()/RAND_MAX)*6.283f; float d=sqrtf((float)rand()/RAND_MAX)*(WORLD_RADIUS-100);
foods[idx].pos=Vec2(cosf(a)*d,sinf(a)*d); foods[idx].radius=4+(rand()%7); if(rand()%10==0) foods[idx].radius=11;
int c=rand()%7; if(c==0){foods[idx].r=255;foods[idx].g=50;foods[idx].b=80;} else if(c==1){foods[idx].r=255;foods[idx].g=200;foods[idx].b=0;} else if(c==2){foods[idx].r=0;foods[idx].g=220;foods[idx].b=100;} else if(c==3){foods[idx].r=0;foods[idx].g=180;foods[idx].b=255;} else if(c==4){foods[idx].r=200;foods[idx].g=0;foods[idx].b=255;} else if(c==5){foods[idx].r=255;foods[idx].g=120;foods[idx].b=0;} else{foods[idx].r=255;foods[idx].g=255;foods[idx].b=255;}
foods[idx].pulse=((float)rand()/RAND_MAX)*6.28f; foods[idx].active=true;
}
void Game::spawnParticle(float x,float y,Uint8 r,Uint8 g,Uint8 b){
for(int i=0;i<MAX_PARTICLES;i++){ if(!particles[i].active){particles[i].pos=Vec2(x,y); float a=((float)rand()/RAND_MAX)*6.283f; particles[i].vel=Vec2(cosf(a)*2,sinf(a)*2); particles[i].life=1; particles[i].r=r; particles[i].g=g; particles[i].b=b; particles[i].size=2+rand()%4; particles[i].active=true; return;}}
}
void Game::updateRanking(){
int n=0; for(int i=0;i<MAX_SNAKES;i++) if(snakes[i].alive) rankOrder[n++]=i;
for(int i=1;i<n;i++){int k=rankOrder[i]; int j=i-1; while(j>=0 && snakes[rankOrder[j]].length<snakes[k].length){rankOrder[j+1]=rankOrder[j]; j--;} rankOrder[j+1]=k;}
rankCount=n; if(snakes[0].alive){for(int i=0;i<n;i++) if(rankOrder[i]==0){playerRank=i+1;break;}}
}
void Game::run(){
padInfo pi; padData pd; Uint32 last=SDL_GetTicks(); bool startBefore=false;
while(running){ if(g_exitRequested) break; Uint32 fs=SDL_GetTicks(); float dt=(fs-last)/16.666f; if(dt>3)dt=3; if(dt<0.1f)dt=0.1f; last=fs; sysUtilCheckCallback();
float ax=0,ay=0; bool boost=false,start=false; ioPadGetInfo(&pi);
if(pi.status[0]){ioPadGetData(0,&pd); int xa=pd.ANA_L_H-128; int ya=pd.ANA_L_V-128; if(xa<-10||xa>10||ya<-10||ya>10){ax=xa/128.0f; ay=ya/128.0f; if(ax>1)ax=1; if(ax<-1)ax=-1; if(ay>1)ay=1; if(ay<-1)ay=-1;} if(pd.BTN_CROSS||pd.BTN_R2)boost=true; if(pd.BTN_START)start=true; if(pd.BTN_CIRCLE) g_exitRequested=true;}
if(gameState==0){ if(start&&!startBefore) resetPlayer(); }
else if(gameState==1){
updatePlayer(snakes[0],dt,ax,ay,boost);
for(int i=1;i<MAX_SNAKES;i++) if(snakes[i].alive) updateBot(snakes[i],dt);
for(int i=0;i<MAX_SNAKES;i++){ if(!snakes[i].alive) continue; Snake &s=snakes[i]; s.path.insert(s.path.begin(),s.body[0]); while((int)s.path.size()>(int)s.length*2+5) s.path.pop_back(); for(int j=0;j<(int)s.length && j<(int)s.body.size();j++){int id=j*2; if(id<(int)s.path.size()) s.body[j]=s.path[id];} }
updateFoods(dt); updateParticles(dt); updateCamera(dt); checkCollisions(); updateRanking();
if(!snakes[0].alive){deathScore=snakes[0].score; deathLength=(int)snakes[0].length; deathKills=snakes[0].kills; deathRank=playerRank; if(deathScore>bestScore)bestScore=deathScore; gameState=2;}
} else if(gameState==2){ if(start&&!startBefore) resetPlayer(); }
startBefore=start; render(); frameCount++; Uint32 el=SDL_GetTicks()-fs; if(el<16) SDL_Delay(16-el);
}
}
void Game::updatePlayer(Snake &s,float dt,float ax,float ay,bool boost){
if(!s.alive) return; if(ax!=0||ay!=0) s.targetAngle=atan2f(ay,ax);
float d=normAngle(s.targetAngle-s.angle); float mt=s.turnSpeed*dt; if(d>mt)s.angle+=mt; else if(d<-mt)s.angle-=mt; else s.angle=s.targetAngle;
s.boosting=boost&&s.length>20; s.speed=s.boosting?s.boostSpeed:s.baseSpeed;
s.body[0].x+=cosf(s.angle)*s.speed*dt; s.body[0].y+=sinf(s.angle)*s.speed*dt;
if(s.boosting){ s.boostTimer+=dt; if(s.boostTimer>8){s.boostTimer=0; if(s.length>20){s.length-=0.5f;}} spawnParticle(s.body[0].x,s.body[0].y,s.cr,s.cg,s.cb); }
if(!insideWorld(s.body[0].x,s.body[0].y)) killSnake(0,-1);
}
void Game::updateBot(Snake &s,float dt){
if(!s.alive) return; s.aiTimer--; Vec2 head=s.body[0];
float dc=dist(head.x,head.y,0,0);
if(dc>WORLD_RADIUS-400){s.aiTarget=Vec2(0,0); s.aiTimer=30;}
else if(s.aiTimer<=0){ float best=999999; Vec2 bf=head; for(int i=0;i<MAX_FOOD;i++){ if(!foods[i].active) continue; float d=dist(head.x,head.y,foods[i].pos.x,foods[i].pos.y); if(d<best){best=d; bf=foods[i].pos;}} s.aiTarget=bf; s.aiTimer=40+rand()%60; }
bool danger=false; float da=0;
for(int i=0;i<MAX_SNAKES;i++){ if(!snakes[i].alive) continue; Snake &o=snakes[i]; int lim=(int)o.length; if(lim>(int)o.body.size()) lim=o.body.size(); for(int j=5;j<lim;j++){ float d=dist(head.x+cosf(s.angle)*120,head.y+sinf(s.angle)*120,o.body[j].x,o.body[j].y); if(d<o.radius+40){danger=true; da=angleTo(head.x,head.y,o.body[j].x,o.body[j].y); break;}} if(danger) break; }
if(danger){ s.targetAngle=da+1.57f; if(rand()%2==0) s.targetAngle=da-1.57f; } else s.targetAngle=angleTo(head.x,head.y,s.aiTarget.x,s.aiTarget.y);
float diff=normAngle(s.targetAngle-s.angle); float mt=s.turnSpeed*dt; if(diff>mt)s.angle+=mt; else if(diff<-mt)s.angle-=mt; else s.angle=s.targetAngle;
s.boosting=(rand()%1000<8)&&s.length>30; s.speed=s.boosting?s.boostSpeed:s.baseSpeed;
s.body[0].x+=cosf(s.angle)*s.speed*dt; s.body[0].y+=sinf(s.angle)*s.speed*dt;
if(!insideWorld(s.body[0].x,s.body[0].y)) { for(int i=0;i<MAX_SNAKES;i++) if(&snakes[i]==&s) killSnake(i,-1); }
}
void Game::updateFoods(float dt){ for(int i=0;i<MAX_FOOD;i++){ if(!foods[i].active) continue; foods[i].pulse+=0.1f*dt; } }
void Game::updateParticles(float dt){ for(int i=0;i<MAX_PARTICLES;i++){ if(!particles[i].active) continue; particles[i].pos.x+=particles[i].vel.x*dt; particles[i].pos.y+=particles[i].vel.y*dt; particles[i].life-=0.03f*dt; if(particles[i].life<=0) particles[i].active=false; } }
void Game::updateCamera(float dt){ (void)dt; if(!snakes[0].alive) return; cameraX+=(snakes[0].body[0].x-cameraX)*0.1f; cameraY+=(snakes[0].body[0].y-cameraY)*0.1f; zoom=1.0f; }
void Game::checkCollisions(){
for(int i=0;i<MAX_SNAKES;i++){ if(!snakes[i].alive) continue; Snake &s=snakes[i]; Vec2 h=s.body[0];
for(int f=0;f<MAX_FOOD;f++){ if(!foods[f].active) continue; if(dist(h.x,h.y,foods[f].pos.x,foods[f].pos.y)<s.radius+foods[f].radius){ eatFood(s,f); } }
for(int j=0;j<MAX_SNAKES;j++){ if(i==j||!snakes[j].alive) continue; Snake &o=snakes[j]; int lim=(int)o.length; if(lim>(int)o.body.size()) lim=o.body.size(); for(int k=8;k<lim;k++){ if(dist(h.x,h.y,o.body[k].x,o.body[k].y)<s.radius*0.6f+o.radius*0.6f){ killSnake(i,j); break; } } if(!s.alive) break; }
}
}
void Game::eatFood(Snake &s,int idx){ foods[idx].active=false; s.length+=0.4f; addScore(s,10); spawnParticle(foods[idx].pos.x,foods[idx].pos.y,foods[idx].r,foods[idx].g,foods[idx].b); spawnFood(idx); if((int)s.body.size()<(int)s.length){ s.body.push_back(s.body.back()); s.path.push_back(s.path.back()); } }
void Game::killSnake(int idx,int killer){
Snake &s=snakes[idx]; if(!s.alive) return; s.alive=false; if(killer>=0&&killer<MAX_SNAKES) snakes[killer].kills++; if(killer>=0) addScore(snakes[killer],100);
for(int i=0;i<(int)s.body.size();i+=2){ for(int f=0;f<MAX_FOOD;f++){ if(!foods[f].active){ foods[f].pos=s.body[i]; foods[f].radius=6; foods[f].r=s.cr; foods[f].g=s.cg; foods[f].b=s.cb; foods[f].active=true; break; } } }
if(idx!=0){ float a=((float)rand()/RAND_MAX)*6.283f; float d=500+((float)rand()/RAND_MAX)*3000; spawnSnake(snakes[idx],false,cosf(a)*d,sinf(a)*d); }
}
void Game::addScore(Snake &s,int v){ s.score+=v; }
float Game::dist(float x1,float y1,float x2,float y2){ float dx=x2-x1,dy=y2-y1; return sqrtf(dx*dx+dy*dy); }
float Game::angleTo(float x1,float y1,float x2,float y2){ return atan2f(y2-y1,x2-x1); }
float Game::normAngle(float a){ while(a>3.14159f)a-=6.283f; while(a<-3.14159f)a+=6.283f; return a; }
Vec2 Game::worldToScreen(float wx,float wy){ return Vec2((wx-cameraX)*zoom+screen->w/2,(wy-cameraY)*zoom+screen->h/2); }
bool Game::insideWorld(float x,float y){ return dist(x,y,0,0)<WORLD_RADIUS; }