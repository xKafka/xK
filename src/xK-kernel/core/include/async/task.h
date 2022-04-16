//
//

#ifndef XGK_TASK_H
#define XGK_TASK_H

#include <type_traits>
#include <stdexcept>

namespace xgk::core::async {
    template<typename>
    class Task;

    template<typename Return, typename ... Args>
    class Task<Return(Args ...)> {
        template<typename Function>
        constexpr static bool CanBeEmpty =
                std::is_pointer_v<std::decay_t<Function>> ||
                std::is_member_pointer_v<std::decay_t<Function>> ||
                std::is_same_v<std::decay_t<Function>, std::decay_t<Return(Args ...)>>;

        /** Returns whether a task is empty */
        template<typename Function>
        constexpr static auto isEmpty(const Function &function) -> std::enable_if_t<CanBeEmpty<Function>, bool>
        { return !function; }

        template<typename Function>
        constexpr static auto isEmpty(const Function &function) -> std::enable_if_t<!CanBeEmpty<Function>, bool>
        { return false; }

        struct Concept {
            void (*destructor)(void *);
            void (*moveConstructor)(void *, void *) noexcept;
            const std::type_info &(*targetType)() noexcept;
            void *(*function)(void *) noexcept;
            const void *(*constFunction)(const void *) noexcept;
        };

        using InvokeHandler = Return (*)(void *, Args ...);

        template<typename Function, bool IsSmall>
        struct Model;

        /** Small size optimization */
        template<typename Function>
        struct Model<Function, true> {
            template<typename FromFunction>
            Model(FromFunction &&function) : m_function(std::forward<FromFunction>(function))
            {}
            Model(Model &&) noexcept = delete;

            static void destructor(void *self)
            { static_cast<Model *>(self)->~Model(); }
            static void moveConstructor(void *self, void *into) noexcept
            { new(into) Model(std::move(static_cast<Model *>(self)->m_function)); }

            static const std::type_info &targetType() noexcept
            { return typeid(Function); }

            static void *function(void *self) noexcept
            { return &static_cast<Model *>(self)->m_function; }
            static const void *constFunction(const void *self) noexcept
            { return &static_cast<const Model *>(self)->m_function; }

            static Return invoke(void *self, Args ... args)
            { return (static_cast<Model *>(self)->m_function)(std::forward<Args>(args) ...); }

            static constexpr Concept vtable = {
                    .destructor = destructor,
                    .moveConstructor = moveConstructor,
                    .targetType = targetType,
                    .function = function,
                    .constFunction = constFunction,
            };
            static constexpr InvokeHandler invokeHandler = invoke;

            Function m_function;
        };

        template<typename Function>
        struct Model<Function, false> {
            template<typename FromFunction>
            Model(FromFunction &&function)
                    : m_function(std::make_unique<Function>(std::forward<FromFunction>(function)))
            {}
            Model(Model &&) noexcept = default;

            static void destructor(void *self)
            { static_cast<Model *>(self)->~Model(); }
            static void moveConstructor(void *self, void *into) noexcept
            { new(into) Model(std::move(static_cast<Model *>(self))); }

            static const std::type_info &targetType() noexcept
            { return typeid(Function); }

            static void *function(void *self) noexcept
            { return &static_cast<Model *>(self)->m_function.get(); }
            static const void *constFunction(const void *self) noexcept
            { return &static_cast<const Model *>(self)->m_function.get(); }

            static Return invoke(void *self, Args ... args)
            { return (*static_cast<Model *>(self)->m_function)(std::forward<Args>(args) ...); }

            static constexpr Concept vtable = {
                    .destructor = destructor,
                    .moveConstructor = moveConstructor,
                    .targetType = targetType,
                    .function = function,
                    .constFunction = constFunction,
            };
            static constexpr InvokeHandler invokeHandler = invoke;

            std::unique_ptr<Function> m_function;
        };

        struct DefaultConcept {
            static void destructor(void *)
            {}
            static void moveConstructor(void *, void *) noexcept
            {}
            static const std::type_info &targetType() noexcept
            { return typeid(void); }
            static void *function(void *self) noexcept
            { return nullptr; }
            static const void *constFunction(const void *self) noexcept
            { return nullptr; }

            static Return invoke(void *self, Args ... args)
            { throw std::bad_function_call(); }

            static constexpr Concept vtable = {
                    .destructor = destructor,
                    .moveConstructor = moveConstructor,
                    .targetType = targetType,
                    .function = function,
                    .constFunction = constFunction,
            };
            static constexpr InvokeHandler invokeHandler = invoke;
        };

        static constexpr auto
                maxAlign = alignof(std::max_align_t),
                maxSmallSize = std::max(maxAlign * 2, sizeof(void *) * 8) -
                               std::max(maxAlign, sizeof(void *) * 2);

    public:
        using result_type = Return;

        constexpr Task() noexcept = default;
        constexpr Task(std::nullptr_t) noexcept: Task()
        {}
        Task(const Task &) = delete;
        Task(Task &&from) noexcept
                : m_vtable(from.m_vtable), m_invokeHandler(from.m_invokeHandler)
        { m_vtable->moveConstructor(&from.m_model, &m_model); }

        template<typename Function, std::enable_if_t<!std::is_same_v<std::decay_t<Function>, Task>, bool> = true>
        Task(Function &&function)
        {
            using SmallModel = Model<std::decay_t<Function>, true>;
            using LargeModel = Model<std::decay_t<Function>, false>;
            using ModelType = std::conditional_t<
                    sizeof(SmallModel) <= maxSmallSize &&
                    alignof(SmallModel) <= alignof(decltype(m_model)),
                    SmallModel,
                    LargeModel
            >;

            if (isEmpty(function))
                return;

            new(&m_model) ModelType(std::forward<Function>(function));
            m_vtable = &ModelType::vtable;
            m_invokeHandler = &ModelType::invokeHandler;
        }

        ~Task() noexcept
        { m_vtable->destructor(&m_model); }

        Task &operator=(const Task &) = delete;
        Task &operator=(Task &&from) noexcept
        {
            m_vtable->destructor(&m_model);
            m_vtable = from.m_vtable;
            m_invokeHandler = from.m_invokeHandler;
            m_vtable->moveConstructor(&from.m_model, &m_model);
            return *this;
        }
        Task &operator=(std::nullptr_t) noexcept
        { return *this = Task(); }

        template<class Function>
        Task &operator=(Function &&function)
        { return *this = task(std::forward<Function>(function)); }

        void swap(Task &with) noexcept
        { std::swap(*this, with); }

        explicit operator bool() const
        { return m_vtable->constFunction(&m_model) != nullptr; }

        [[nodiscard]]
        const std::type_info &target_type() const noexcept
        { return m_vtable->targetType(); }

        template<typename T>
        T *target()
        {
            return target_type() == typeid(T) ?
                   static_cast<T *>(m_vtable->function(&m_model)) :
                   nullptr;
        }
        template<typename T>
        const T *target() const
        {
            return target_type() == typeid(T) ?
                   static_cast<T *>(m_vtable->constFunction(&m_model)) :
                   nullptr;
        }

        template<typename ... CallArgs>
        auto operator()(CallArgs &&... callArgs)
        { return m_invoke(&m_model, std::forward<CallArgs>(callArgs) ...); }

        friend inline void swap(Task &left, Task &right)
        { return left.swap(right); }
        friend inline bool operator==(const Task &left, std::nullptr_t)
        { return !static_cast<bool>(left); }
        friend inline bool operator==(std::nullptr_t, const Task &right)
        { return !static_cast<bool>(right); }
        friend inline bool operator!=(const Task &left, std::nullptr_t)
        { return static_cast<bool>(left); }
        friend inline bool operator!=(std::nullptr_t, const Task &right)
        { return static_cast<bool>(right); }

    private:
        const Concept *m_vtable = &DefaultConcept::vtable;
        InvokeHandler m_invokeHandler = &DefaultConcept::invokeHandler;
        std::aligned_storage_t<maxSmallSize> m_model;
    };
}

#endif //XGK_TASK_H
