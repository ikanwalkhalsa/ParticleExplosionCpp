#include<iostream>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<cstring>
#include<SDL2/SDL.h>
using namespace std;


class Screen{
    private:
        SDL_Window *win;
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        Uint32 *buffer;
        Uint32 *b1;
    public:
        const static int WIDTH = 800;
        const static int HEIGHT = 600;
        Screen(){
            SDL_Init(SDL_INIT_EVERYTHING);
            win = SDL_CreateWindow("Particle Fire Explosion",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH,HEIGHT,SDL_WINDOW_SHOWN);
            renderer = SDL_CreateRenderer(win,-1,SDL_RENDERER_PRESENTVSYNC);
            texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STATIC,WIDTH,HEIGHT);
            buffer = new Uint32[WIDTH*HEIGHT];
            b1 = new Uint32[WIDTH*HEIGHT];
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
        void update(){
            SDL_UpdateTexture(texture,NULL,buffer,WIDTH*sizeof(buffer[0]));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer,texture,NULL,NULL);
            SDL_RenderPresent(renderer);
        }
        void clear(){
            memset(buffer,0,WIDTH*HEIGHT*sizeof(buffer[0]));
            update();
        }
        void boxBlur(){
            Uint32 *temp = buffer;
            buffer = b1;
            b1 = temp;
            for(int y = 0; y < HEIGHT; y++)
                for(int x = 0; x < WIDTH; x++){
                    Uint32 red = 0;
                    Uint32 green = 0;
                    Uint32 blue = 0;
                    for(int row=-1; row<=1;row++)
                        for(int col=-1;col<=1;col++){
                            if((x+col)>=0&&(x+col)<WIDTH&&(y+row)>=0&&(y+row)<HEIGHT){
                                Uint32 clr = b1[((y+row)*WIDTH)+(x+col)];
                                red+=(clr&0xFF000000)>>24;
                                green+=(clr&0x00FF0000)>>16;
                                blue+=(clr&0x0000FF00)>>8;
                            }
                        }
                    red/=9;
                    green/=9;
                    blue/=9;
                    setPixal(x,y,red,green,blue);
                }

        }
        void setPixal(int x, int y, Uint8 red, Uint8 green, Uint8 blue){
            if(x>0&&x<WIDTH&&y>0&&y<HEIGHT){
                Uint32 color = 0 | (red << 24) | (green << 16) | (blue << 8) | 0xFF;
                buffer[(y*WIDTH)+x] = color;
            }
        }
        bool processEvent(){
            SDL_Event event;
            while(SDL_PollEvent(&event)){
                if(event.type == SDL_QUIT){
                    return false;
                }
            }
            return true;
        }
        void close(){
            delete [] buffer;
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(win);
            SDL_Quit();
        }
};

class Particle{
    public:
        double x, y, speed, direction;
        Particle():x(0),y(0){
            init();
        }
        void init(){
            x=0;
            y=0;
            direction = (2 * M_PI * rand())/RAND_MAX;
            speed = (0.04 * rand())/RAND_MAX;
            speed*=speed;
        }
        void update(int interval){
            direction += interval*0.001;
            x += (speed*cos(direction))*interval;
            y += (speed*sin(direction))*interval;
            if(x<-1||x>1||y<-1||y>1||rand()<RAND_MAX/100){
                init();
            }
        }
};

class Swarm{
    private:
        Particle *particles;
        int lastTime;
    public:
        const static int NPARTICLES = 5000;
        Swarm():lastTime(0){
            particles = new Particle[NPARTICLES];
        }
        ~Swarm(){
            delete [] particles;
        }
        Particle *getParticles(){
            return particles;
        }
        void update(int elasped){
            int interval = elasped - lastTime;
            for(int i = 0; i < NPARTICLES; i++){
                particles[i].update(interval);
            }
            lastTime = elasped;
        }
};

int main(int argc, char *argv[]){

    srand(time(NULL));
    
    Screen sc;
    Swarm swarm;

    while(true){
        if(!sc.processEvent())break;

        int elasped = SDL_GetTicks();
        unsigned char red = (1 + sin(elasped*0.001)) * 127.5;
        unsigned char green = (1 + sin(elasped*0.002)) * 127.5;
        unsigned char blue = (1 + sin(elasped*0.003)) * 127.5;

        swarm.update(elasped);
        sc.boxBlur();
        
        Particle *particles = swarm.getParticles();
        for(int i = 0; i < swarm.NPARTICLES; i++){
            int x = (particles[i].x+1)*(sc.WIDTH/2);
            int y = (particles[i].y+1)*(sc.WIDTH/2) - (sc.WIDTH-sc.HEIGHT)/2;
            sc.setPixal(x,y,red,green,blue);
        }
        
        sc.update();    
    }

    sc.close();
    
    return EXIT_SUCCESS;
}