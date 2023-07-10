#ifndef VAL_RELEASABLE_HPP
#define VAL_RELEASABLE_HPP

namespace VAL {
    class ValInstance;

    class ValReleasable {
    public:
        // Allows this object to release Vulkan resources associated with it
        virtual void release(ValInstance *p_val_instance);
    };
}

#endif
