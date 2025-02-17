#include <memory>
#include <mutex>
#include <unordered_map>


template< typename Key, typename Value > class LruCache;

template< typename Key, typename Value > 
class ListNode{
    public:
        ListNode(Key key, Value value): key_(key), value_(value), accessCount_(0), prev(nullptr), next(nullptr){}

        Key getKey() const{
            return key_;
        }

        Value getValue() const{
            return value_;
        }

        void setValue(const Value& value){
            this->value_ = value;
        }

        int getAccessCount() const{
            return accessCount_;
        }

        void increaseAccessCount(){
            accessCount_++;
        }

    private:
        Key key_;
        Value value_;
        int accessCount_;

        using ListNodeType = ListNode<Key, Value>;
        std::shared_ptr<ListNodeType> prev;
        std::shared_ptr<ListNodeType> next;

        friend class LruCache<Key, Value>;
};

template< typename Key, typename Value > 
class LruCache{
    public:
        using ListNodeType = ListNode<Key, Value>;
        using NodePtr = std::shared_ptr<ListNodeType>;
        using NodeMap = std::unordered_map<Key, NodePtr>;

    private:
        std::mutex mutex_;
        int capacity_;
        NodeMap nodeMap_;
        NodePtr nodePtr_;
        ListNodeType head_;
        ListNodeType tail_;

    public:
        LruCache(int capacity): capacity_(capacity) {
            initializeList();
        }

        ~LruCache() {}

        void put(Key key, Value value) {
            if(capacity_ == 0){
                return;
            }

            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if(it != nodeMap_.end()){
                updateExistNode(it->second, value);
                return;
            }

            addNewNode(key, value);
        }

        bool get(Key key, Value& value) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if(it != nodeMap_.end() ){
                // 刚被访问过，所以移动到最近访问过的节点
                moveMostRecentNode(it->second);;
                value = it->second->getValue;
                return true;
            }

            return false;
        }

        Value get(Key key) {
            Value value;
            get(key, value);
            return value;
        }

        void remove(Key key) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if(it != nodeMap_.end()){
                removeNode(it->second);
            }
        }

    private:
        void initializeList(){
            head_ = std::make_shared<ListNodeType>(Key(), Value());
            tail_ = std::make_shared<ListNodeType>(Key(), Value());

            head_.next = tail_;
            tail_.prev = head_;
        }

        void updateExistNode(NodePtr node, const Value& value){
            node->setValue(value);
            moveMostRecentNode(node);
        }

        void addNewNode(const Key& key, const Value& value){
            if (nodeMap_.size() >= capacity_){
                removeLeastRecentNode();
            }

            NodePtr newNode = std::make_shared<ListNodeType>(key, value);
            insertNode(newNode);
            nodeMap_[key] = newNode;

            return;
        }

        void removeNode(NodePtr node){
            node->prev->next = node->next;
            node->next->prev = node->prev;

            return;
        }

        void insertNode(NodePtr node){
            node->next = tail_;
            node->prev = tail_->prev;
            tail_->prev->next = node;
            tail_->prev = node;

            return;
        }

        void moveMostRecentNode(NodePtr node){
            removeNode(node);
            insertNode(node);

            return;
        }

        void removeLeastRecentNode(){
            //因为最近访问的节点在尾部，所以头部节点是最近未访问的
            NodePtr lastRecentNode = head_.next;
            removeNode(lastRecentNode);
            nodeMap_.erase(lastRecentNode->getKey());

            return;
        }



};