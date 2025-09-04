# DIVISIBLE REGEX GENERATOR

---

对于正整数 $N$ 和 $M$, 生成可匹配形如 $A + B \equiv M (mod N)$ 的二进制表达式 `A+B` 的正则表达式.

## 编译

```sh
make
```

## 运行

```sh
make run RUNARGS="[N] [M]"
```

或

```sh
./debug/gen.exe [N] [M]
```

## 效果演示

下面展示被 5 除余 3 的情况:

![控制台输出](doc/pic/CLI-effect.png)

![匹配效果](doc/pic/match-effect.png)

## 碎碎念

本质上, 任意由加减乘除和括号生成的表达式都是可匹配的, 并且可以通过传入一个模板参数来生成对应表达式. 但这涉及到递归生成子图, 而我的代码乱写的工程性极差, 就懒得实现了...
