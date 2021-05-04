// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <utility>
#include <thread>
#include <mutex> 
namespace EDA {
  namespace Concurrent {
    template <std::size_t B, typename Type>
    class BLinkTree {
    public:
      typedef Type data_type;
      class Bplusnode{
      public:  
        bool leaf;
        Type* key;
        Type size;
        Type* fordward;
        std::mutex mtx_write;
        std::mutex mtx_read;
        Bplusnode** ptr;
        Bplusnode(): key(new Type[B]), ptr(new Bplusnode*[B+1]), fordward(nullptr){};
      };
      Bplusnode* root;
      void insert_nodo_interno(Type value,Bplusnode* ayuda,Bplusnode* child){
        mtx_write.lock();
        if(ayuda->size < B){
          int i=0;
          while(value > ayuda->key[i] && i < ayuda->size) i++;
          for(int j = ayuda->size; j > i; j--) ayuda->key[j]=ayuda->key[j-1];
          for(int j = ayuda->size+1; j > i+1; j--) ayuda->ptr[j]=ayuda->ptr[j-1];

          ayuda->key[i]=value;
          ayuda->size++;
          ayuda->ptr[i+1]=child;
          nleaf->fordward = ayuda;
        }
        else{
          Bplusnode* ninternal = new Bplusnode;
          int virtualkey[B+1];
          Bplusnode* virtualptr[B+2];
          for(int i=0;i < B;i++) virtualkey[i]= ayuda->key[i];
          for(int i=0;i<B+1;i++) virtualptr[i]=ayuda->ptr[i];

          int i=0,j;
          while (value > virtualkey[i] && i < B) i++;

          for(int j = B; j > i; j--) virtualkey[j]= virtualkey[j-1];

          virtualkey[i]=value;
          for(int j = B+1; j > i+1; j--) virtualptr[j]= virtualptr[j-1];

          virtualptr[i+1]=child;
          nleaf->fordward = ayuda;
          ninternal->leaf=false;
          ayuda->size=(B+1)/2;
          ninternal->size=B - (B+1)/2;
          for(i=0,j=ayuda->size+1;i<ninternal->size;i++,j++) ninternal->key[i]=virtualkey[j];

          for(i=0,j=ayuda->size+1;i<ninternal->size+1;i++,j++) ninternal->ptr[i]=virtualptr[j];
          if(ayuda == root){
            Bplusnode* nroot = new Bplusnode;
            nroot->key[0]= ayuda->key[ayuda->size];
            nroot->ptr[0]=ayuda;
            nroot->ptr[1]=ninternal;
            nroot->leaf=false;
            nroot->size=1;
            root=nroot;
          }
          else{
            insert_nodo_interno(ayuda->key[ayuda->size],Buscar_padre(root,ayuda),ninternal);
            mtx_write.unlock();
          }
        }
      }
      Bplusnode* Buscar_padre(Bplusnode* ayuda, Bplusnode* child){
        mtx_read.lock();
        Bplusnode* padre;
        if(ayuda->leaf || (ayuda->ptr[0])->leaf){
          mtx_read.unlock();
          return nullptr;
        }
        for(int i=0; i<ayuda->size+1;i++){
          if(ayuda->ptr[i]==child){
            padre=ayuda;
            mtx_read.unlock();
            return padre;
          }
          else{
            padre=Buscar_padre(ayuda->ptr[i],child);
            if(padre != nullptr){
              mtx_read.unlock();
              return padre;
            }
          }
        }
        mtx_read.unlock();
        return padre;
      }
      BLinkTree() {
        root = nullptr;
      }

      ~BLinkTree() {}

      std::size_t size() const {}

      bool empty() const {
        if (root == nullptr) return 1;
        return 0;
      }

      bool search(const data_type& value) const {
        mtx_read.lock();
        if (root == nullptr){
          return 0;
        }
        else{
          Bplusnode* ayuda = root;
          while(ayuda->leaf == false){
            for(int i=0;i<ayuda->size;i++){
              if(value < ayuda->key[i]){
                ayuda = ayuda->ptr[i];
                break;
              }
              if(i == ayuda->size-1){
                ayuda = ayuda->ptr[i+1];
                break;
              }
            }
          }
          for (int i=0;i<ayuda->size;i++){
            if(ayuda->key[i]==value){
              mtx.unlock();
              return 1;
            }
          }
        }
        mtx.unlock();
        return 0;
      }

      void insert(const data_type& value) {
        mtx_write.lock();
        if(search(value)){
          mtx_write.unlock();
          return;
        }
        if(root == nullptr){
          root = new Bplusnode;
          root->key[0]=value;
          root->leaf=true;
          root->size=1;
          root->fordward = nullptr;
          mtx_write.unlock();
          std::cout<<"elemento insertado exitosamente"<<std::endl;
        }
        
        else{
          Bplusnode* ayuda = root;
          Bplusnode* parent;
          while(ayuda->leaf == false){
            parent = ayuda;
            for(int i=0; i<ayuda->size;i++){
              if(value < ayuda->key[i]){
                ayuda = ayuda->ptr[i];
                break;
              }
              if(i== ayuda->size-1){
                ayuda = ayuda->ptr[i+1];
                break;
              }
            }
          }
          if(ayuda->size < B){
            int i=0;
            while (value > ayuda->key[i] && i < ayuda->size) i++;
            for(int j = ayuda->size; j > i; j--) ayuda->key[j]= ayuda->key[j-1];
            ayuda->key[i] = value;
            ayuda->size++;
            ayuda->ptr[ayuda->size]= ayuda->ptr[ayuda->size-1];
            ayuda->ptr[ayuda->size-1] = nullptr;
            mtx_write.unlock();
            std::cout<<"elemento insertado exitosamente"<<std::endl;
          }
          else{
            std::cout<<"elemento insertado exitosamente"<<std::endl;
            std::cout<<"overflow.... spliting child"<<std::endl;
            Bplusnode* nleaf = new Bplusnode;
            int virtual_node[B + 1];
            for(int i=0;i<B;i++) virtual_node[i]=ayuda->key[i];
            int j,i=0;
            while(value > virtual_node[i] && i < B) i++;
            for(int j = B; j > i; j--) virtual_node[j]=virtual_node[j-1];
            virtual_node[i]= value;
            nleaf->leaf = true;
            ayuda->size = (B+1)/2;
            nleaf->size = B + 1 - (B+1)/2;
            ayuda->ptr[ayuda->size]=nleaf;
            nleaf->ptr[nleaf->size]=ayuda->ptr[B];
            ayuda->ptr[B]=nullptr;
            nleaf->fordward = ayuda;
            for(int i=0; i<ayuda->size;i++) ayuda->key[i]=virtual_node[i];
            for(int i=0, j = ayuda->size; i<nleaf->size;i++,j++) nleaf->key[i]=virtual_node[j];
            if(ayuda == root){
              Bplusnode* nroot= new Bplusnode;
              nroot->key[0]=nleaf->key[0];
              nroot->ptr[0]= ayuda;
              nroot->ptr[1]=nleaf;
              nroot->leaf=false;
              nroot->size=1;
              root = nroot;
              mtx_write.unlock();
              std::cout<<"Nueva raiz creada\n";
            }
            else{
              insert_nodo_interno(nleaf->key[0],parent,nleaf);
              mtx_write.unlock();
            }
          }
        }
      }

      void remove(const data_type& value) {}

    private:
      data_type* data_;
    };

  }  
}  

#endif 
sasas



void LogicEngine::VerifyColision() {
  //crear un instancia momentanea futura para analizar colisiones y luego decidir
  bool flag_aircraftCollide_0 = false;
  bool flag_aircraftCollide_1 = false;
  bool flag_aircraftCollide_2 = false;
  bool flag_aircraftCollide_3 = false;
  bool flag_aircraftCollide_4 = false;

  //std::vector<bool> flags_aicraft_colliders;

  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
   flag_aircraftCollide_0 = *it->box_collider.intersects(aircrafts_[0].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_1 = *it->box_collider.intersects(aircrafts_[1].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_2 = *it->box_collider.intersects(aircrafts_[2].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_3 = *it->box_collider.intersects(aircrafts_[3].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_4 = *it->box_collider.intersects(aircrafts_[4].box_collider);
  }

  bool flag_bulletCollide_0 = false;
  bool flag_bulletCollide_1 = false;
  bool flag_bulletCollide_2 = false;
  bool flag_bulletCollide_3 = false;
  bool flag_bulletCollide_4 = false;

  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
   flag_bulletCollide_0 = *it->box_collider.intersects(bullets_[0].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_1 = *it->box_collider.intersects(bullets_[1].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_2 = *it->box_collider.intersects(bullets_[2].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_3 = *it->box_collider.intersects(bullets_[3].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_4 = *it->box_collider.intersects(bullets_[4].box_collider);
  }
  
}void LogicEngine::VerifyColision() {
  //crear un instancia momentanea futura para analizar colisiones y luego decidir
  bool flag_aircraftCollide_0 = false;
  bool flag_aircraftCollide_1 = false;
  bool flag_aircraftCollide_2 = false;
  bool flag_aircraftCollide_3 = false;
  bool flag_aircraftCollide_4 = false;

  //std::vector<bool> flags_aicraft_colliders;

  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
   flag_aircraftCollide_0 = *it->box_collider.intersects(aircrafts_[0].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_1 = *it->box_collider.intersects(aircrafts_[1].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_2 = *it->box_collider.intersects(aircrafts_[2].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_3 = *it->box_collider.intersects(aircrafts_[3].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = aircrafts_.begin(); it != aircrafts_.end(); it++){
    flag_aircraftCollide_4 = *it->box_collider.intersects(aircrafts_[4].box_collider);
  }

  bool flag_bulletCollide_0 = false;
  bool flag_bulletCollide_1 = false;
  bool flag_bulletCollide_2 = false;
  bool flag_bulletCollide_3 = false;
  bool flag_bulletCollide_4 = false;

  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
   flag_bulletCollide_0 = *it->box_collider.intersects(bullets_[0].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_1 = *it->box_collider.intersects(bullets_[1].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_2 = *it->box_collider.intersects(bullets_[2].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_3 = *it->box_collider.intersects(bullets_[3].box_collider);
  }
  for(list<PlayerRepresentation>::iterator it = bullets_.begin(); it != bullets_.end(); it++){
    flag_bulletCollide_4 = *it->box_collider.intersects(bullets_[4].box_collider);
  }
  
}
