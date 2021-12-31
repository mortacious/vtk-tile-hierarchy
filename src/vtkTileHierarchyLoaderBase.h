//
// Created by mortacious on 12/30/21.
//

#pragma once
#include <vtkSmartPointer.h>
#include <vtkWrappingHints.h>
#include <vtkObject.h>
#include <vtkSetGet.h>
#include <vtkCommand.h>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "minMaxHeap.h"
#include "lruCache.h"
#include "vtkTileHierarchyModule.h" // For export macro

class vtkMapper;
class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;

// custom specialization of std::hash for node pointers
template<>
struct std::hash<vtkTileHierarchyNodePtr> {
    std::size_t operator()(vtkTileHierarchyNodePtr const &s) const noexcept;
};

struct vtkTileHierarchyLoaderRenderState {
    vtkTileHierarchyLoaderRenderState() = default;
    virtual ~vtkTileHierarchyLoaderRenderState() = default;
};

VTK_WRAPEXCLUDE class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyLoaderBase: public vtkObject {
public:
    vtkTypeMacro(vtkTileHierarchyLoaderBase, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    virtual void Initialize();
    void UnscheduleAll();
    void ScheduleForLoading(vtkTileHierarchyNodePtr& node, float priority);
    vtkTileHierarchyNodePtr PopNextNode();

    vtkGetMacro(MaxInQueue, unsigned int);
    vtkSetMacro(MaxInQueue, unsigned int);

    void SetCacheSize(size_t cs) {
        Cache.max_cache_size(cs);
    }

    size_t GetCacheSize() const {
        return Cache.cache_size();
    }

    vtkTileHierarchyNodePtr GetRootNode();
    void SetRootNode(vtkTileHierarchyNode* root_node);

    void LoadNode(vtkTileHierarchyNodePtr& node) {
        LoadNode(node, false);
    }

    virtual void LoadNode(vtkTileHierarchyNodePtr& node, bool recursive);

    void UnloadNode(vtkTileHierarchyNodePtr& node) {
        UnloadNode(node, false);
    }

    virtual std::unique_ptr<vtkTileHierarchyLoaderRenderState> PreRender() {
        return std::make_unique<vtkTileHierarchyLoaderRenderState>();
    };

    virtual void PostRender(std::unique_ptr<vtkTileHierarchyLoaderRenderState> state) {
    };

    void UnloadNode(vtkTileHierarchyNodePtr& node, bool recursive);

    virtual void FetchNode(vtkTileHierarchyNodePtr node) = 0;

    void SetNodeLoadedCallBack(const std::function<void()>& func);

protected:
    vtkTileHierarchyLoaderBase();
    ~vtkTileHierarchyLoaderBase() noexcept override;

    bool TryGetNodeFromCache(vtkTileHierarchyNodePtr& node);
    void InvokeNodeLoaded() const;

    virtual void Shutdown();

    std::atomic_bool Stop;
    mutable std::mutex Mutex;
    std::condition_variable Cond;

    using HeapElement = std::pair<vtkTileHierarchyNodePtr, float>;

    struct Compare
    {
        bool operator()(const HeapElement& e1, const HeapElement& e2) const
        {
            return e1.second < e2.second;
        }
    };
    minmax::MinMaxHeap<HeapElement, std::vector<HeapElement>, Compare> NeedToLoad;

    unsigned int MaxInQueue;

    vtkTileHierarchyNodePtr RootNode;

    struct TileTreeNodeSize {
        size_t operator()(const vtkTileHierarchyNodePtr & k , const std::pair<vtkSmartPointer<vtkMapper>, size_t>& v) const;
    };

    using LRUCacheType = LRUCache<vtkTileHierarchyNodePtr, std::pair<vtkSmartPointer<vtkMapper>, size_t>, TileTreeNodeSize>;
    LRUCacheType Cache;

    std::mutex CacheMutex;

    std::function<void()> Func;
private:
    vtkTileHierarchyLoaderBase(const vtkTileHierarchyLoaderBase&) = delete;
    void operator=(const vtkTileHierarchyLoaderBase&) = delete;
};

