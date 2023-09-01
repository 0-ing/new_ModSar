## 需要遵循一定的编码规范
1. 文件名 全部小写 有意义的单词用_隔离 name1_name2.cpp 或者 name2_name3.hpp
    - *.hpp 代表有class 实现体的文件 一般是模板类所在的文件
    - *.h 代码只有class 定义的文件 一般是没有是先提的文件
    - *.cpp 代码有相关的class 实现
2. 头文件的实现规则
    ``` c++
    #ifndef EVENT_PROXY_HPP_ // 一般文件名的定义 
    #define EVENT_PROXY_HPP_
    // 具体的定义
    #endif // EVENT_PROXY_HPP_ // 必须尾随宏
3. 关于头注释
    ```
    /**
     * 头文件注释
     * copyright 说明
    **/
4. class 的命名方式
    采用驼峰命名 首字符大写
    ``` c++
    class ActionSampleHello {};
    ```
5. function 命名方式
    public 函数 首字符大写
    private / protected 函数 首字符小写
6. 头文件命令 
    驼峰命名前面 + I IInterface
    
7. field 命名方式
    public 首字符小写
    private xx_ 首字符小写 且尾随_'
    ``` c++
    /**
     * \brief 类说明
     */
    class ActionSampleHello : public IHello {
    public:
        /**
         * \brief 函数说明
         * \return  void 不返回值
         */
        void Action1();
    private:
        /**
         * \brief 函数说明
         * \param a int 输入参数说明  
         * \return int 1 代表成功 0 代表失败
         */
        int action2(int a);
    private:
        int action1_; // 说明2
    public:
        int action; // 说明1
    };

8. namespace 定义 小写 依据有效目录
    ``` c++
    namespace xx
    {
        namespace yy
        {

        }; // namespace yy
    }; // namespace xx
    ```