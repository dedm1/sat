#include <QCoreApplication>
#include "iostream"
#include "QFile"
#include "QTextStream"
#include "BBV.h"
#include <string>
#include <cstring>
#include "NodeBoolTree.h"
#include "boolinterval.h"
#include "QStack"
#include "boolequation.h"
#include "stack"
#include<QDebug>


void CutRoot(BoolInterval **DNF, BoolInterval *finded_root,int dnfsize)
{
    for(int z = 0;z<finded_root->length();z++)
    {
        if(finded_root->getValue(z) != '-')
        {
            char val = finded_root->getValue(z);
            finded_root->setValue('-', z);          
            for(int i=0;i<dnfsize;i++)
            {
                if(!DNF[i]->isOrthogonal(*finded_root))
                {
                    finded_root->setValue(val, z);
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList full_file_list;
    QList<QStringList> Elements;
    std::string filepath;
    QStringList inputs;
    qDebug("Input file path...");// << "Input file path...\n";f
    std::cout << " file path...\n";
    //std::cin >> filepath;
   filepath="/home/dedm/data/sat1/SATlab1/laba_SAT1/dnfRnd_1.pla";
    QFile file(QString::fromUtf8(filepath.c_str()));
    //считываем весь файл
    if((file.exists())&&(file.open(QIODevice::ReadOnly)))
    {
        std::cout << "enter file path...\n";
        while (!file.atEnd())
        {
            full_file_list<<file.readLine().replace("\r\n","").simplified();
        }

        int dnfSize = full_file_list.length();
        BoolInterval **DNF = new BoolInterval*[dnfSize];

        for(int i=0;i<dnfSize;i++) // Заполняем массив ДНФ
        {            
            QString strv = full_file_list[i];
            QByteArray v = strv.toUtf8();
            BBV vec(v.data());
            QString strd = "";
            for(int p=0;p<strv.length();p++)
            {
                if(strv[p] == '-')
                {
                  strd += "1";
                }
                else
                {
                  strd += "0";
                }
            }
            QByteArray d = strd.toUtf8();
            BBV dnc(d.data());
            DNF[i] = new BoolInterval(vec, dnc);
        }
        QString rootvec = "";
        QString rootdnc = "";
        for(int i = 0; i < full_file_list[0].length();i++)
        {
            rootvec += "0";
            rootdnc += "1";
        }
        QByteArray v = rootvec.toUtf8();
        BBV vec(v.data());
        QByteArray d = rootdnc.toUtf8();
        BBV dnc(d.data());
        BoolInterval *root = new BoolInterval(vec,dnc);

        BoolEquation *boolequation = new BoolEquation(DNF, root, dnfSize, dnfSize, vec);

        // Алгоритм поиска корня. Работаем всегда с верхушкой стека.
        // Шаг 1. Правила выполняются? Нет - Ветвление Шаг 5. Да - Упрощаем Шаг 2.
        // Шаг 2. Строки закончились? Нет - Шаг1, Да - Корень найден? Да - Успех КОНЕЦ, Нет - Шаг 3.
        // Шаг 3. Кол-во узлов в стеке > 1? Нет - Корня нет КОНЕЦ, Да - Шаг 4.
        // Шаг 4. Текущий узел выталкиваем из стека, попадаем в новый узел. У нового узла lt rt отличны от NULL? Нет - Шаг 1. Да - Шаг 3.
        // Шаг 5. Выбор компоненты ветвления, создание двух новых узлов, добавление их в стек сначала с 1 потом с 0. Шаг 1.

        // Алгоритм CheckRules.
        // Цикл по строкам ДНФ.
        // 1. Проверка правила 2. Выполнилось? Да - Корня нет, Нет - Идем дальше.
        // 2. Проверка правила 1. Выполнилось? Да - Упрощаем, Нет - Идем дальше.


        // Создаем стек под узлы булева дерева
        // QStack<NodeBoolTree> BoolTree;
        bool rootIsFinded = false;
        stack<NodeBoolTree*> BoolTree;
        NodeBoolTree *startNode = new NodeBoolTree(boolequation);
        BoolTree.push(startNode);

        do
        {
            NodeBoolTree *currentNode(BoolTree.top());
            if(currentNode->lt == nullptr && currentNode->rt == nullptr) // Если вернулись в обработанный узел
            {
                BoolEquation *currentEquation = currentNode->eq;
                bool flag = true;
                // Цикл для упрощения ДНФ по правилам.
                while(flag)
                {
                    int a = currentEquation->CheckRules(); // Проверка выполнения правил
                    switch(a)
                    {
                      case 0: // Корня нет.
                      {
                        BoolTree.pop();
                        flag = false;
                        break;
                      }
                      case 1: // Правило выполнилось, корень найден или продолжаем упрощать.
                      {
                        if(currentEquation->count == 0 || currentEquation->mask.getWeight() == currentEquation->mask.getSize()) // Если кончились строки или столбцы, корень найден.
                        {
                            flag = false;
                            rootIsFinded = true; // Полагаем, что корень найден, проверяем на ортогональность
                            for(int i=0;i<dnfSize;i++)
                            {
                                if(!DNF[i]->isOrthogonal(*currentEquation->root))
                                {
                                    rootIsFinded = false;
                                    BoolTree.pop();
                                    break;
                                }
                            }
                        }
                        break;
                      }
                      case 2: // Правила не выполнились, ветвление.
                      {
                        // Ветвление, создание новых узлов.

                        int indexCol = currentEquation->ChooseColForBranching();
                        char val = currentEquation->ChooseValForBranching(indexCol);

                        BoolEquation *Equation0 = new BoolEquation(*currentEquation);
                        BoolEquation *Equation1 = new BoolEquation(*currentEquation);
                        Equation0->Simplify(indexCol, '0');
                        Equation1->Simplify(indexCol, '1');

                        NodeBoolTree *Node0 = new NodeBoolTree(Equation0);
                        NodeBoolTree *Node1 = new NodeBoolTree(Equation1);
                        currentNode->lt = Node0;
                        currentNode->rt = Node1;
                        if(val == '0')
                           BoolTree.push(Node1);
                        else
                           BoolTree.push(Node0);

                        flag = false;
                        break;
                      }
                    }
                }
            }
            else
            {
                BoolTree.pop();
            }

        } while (BoolTree.size()>1 && !rootIsFinded);

        if(rootIsFinded)
        {

            BoolInterval *finded_root = BoolTree.top()->eq->root;

            CutRoot(DNF,finded_root,dnfSize);

            cout << string(*finded_root);
        }
        else
        {
           cout << "Root is not exists!";
        }

    }
    else
    {
       std::cout << "File does not exists.\n";
    }
    return a.exec();
}
