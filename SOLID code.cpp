#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <memory>

// Базовый класс Plant
class Plant {
public:
    std::string name;

    Plant(const std::string& name) : name(name) {}
    virtual ~Plant() {}

    virtual std::string getType() const = 0;
    virtual std::string toString() const = 0;
    virtual bool compare(int value, const std::string& op) const = 0; // для LSP
    virtual bool compare(double value, const std::string& op) const { return false; } // для кактуса
};

// Класс Tree
class Tree : public Plant {
public:
    int age;

    Tree(const std::string& name, int age) : Plant(name), age(age) {}

    std::string getType() const override { return "Tree"; }

    std::string toString() const override {
        return "Tree: " + name + ", Age: " + std::to_string(age);
    }

    bool compare(int value, const std::string& op) const override {
        return applyCondition(age, value, op);
    }

private:
    bool applyCondition(int a, int b, const std::string& op) const {
        if (op == "=") return a == b;
        if (op == "<") return a < b;
        if (op == ">") return a > b;
        if (op == "<=") return a <= b;
        if (op == ">=") return a >= b;
        return false;
    }
};

// Класс Shrub
class Shrub : public Plant {
public:
    std::string flowering_month;

    Shrub(const std::string& name, const std::string& flowering_month)
        : Plant(name), flowering_month(flowering_month) {}

    std::string getType() const override { return "Shrub"; }

    std::string toString() const override {
        return "Shrub: " + name + ", Flowering Month: " + flowering_month;
    }

    bool compare(int value, const std::string& op) const override {
        return false; // Для кустарника нет логики сравнения по возрасту
    }

    bool compare(const std::string& month, const std::string& op) const {
        return applyCondition(flowering_month, month, op);
    }

private:
    bool applyCondition(const std::string& a, const std::string& b, const std::string& op) const {
        if (op == "=") return a == b;
        return false; // Другие операции для строк не реализованы
    }
};

// Класс Cactus
class Cactus : public Plant {
public:
    double spine_length;

    Cactus(const std::string& name, double spine_length)
        : Plant(name), spine_length(spine_length) {}

    std::string getType() const override { return "Cactus"; }

    std::string toString() const override {
        return "Cactus: " + name + ", Spine Length: " + std::to_string(spine_length);
    }

    bool compare(int value, const std::string& op) const override {
        return false; // Для кактуса нет логики сравнения по возрасту
    }

    bool compare(double value, const std::string& op) const override {
        return applyCondition(spine_length, value, op);
    }

private:
    bool applyCondition(double a, double b, const std::string& op) const {
        if (op == "=") return a == b;
        if (op == "<") return a < b;
        if (op == ">") return a > b;
        if (op == "<=") return a <= b;
        if (op == ">=") return a >= b;
        return false;
    }
};

// Интерфейс для команд
class ICommand {
public:
    virtual void execute() = 0;
    virtual ~ICommand() {}
};

// Команда для добавления растений
class AddPlantCommand : public ICommand {
private:
    std::shared_ptr<Plant> plant;
    std::vector<std::shared_ptr<Plant>>& plants;

public:
    AddPlantCommand(std::shared_ptr<Plant> p, std::vector<std::shared_ptr<Plant>>& cont)
        : plant(p), plants(cont) {}

    void execute() override {
        plants.push_back(plant);
    }
};

// Команда для удаления растений
class RemovePlantCommand : public ICommand {
private:
    std::vector<std::shared_ptr<Plant>>& plants;
    std::string condition;

public:
    RemovePlantCommand(std::vector<std::shared_ptr<Plant>>& cont, const std::string& cond)
        : plants(cont), condition(cond) {}

    void execute() override {
        std::istringstream iss(condition);
        std::string type;
        iss >> type;

        if (type == "age") {
            std::string op;
            int age;
            iss >> op >> age;
            plants.erase(std::remove_if(plants.begin(), plants.end(),
                [&op, age](const std::shared_ptr<Plant>& plant) {
                    return plant->getType() == "Tree" && plant->compare(age, op);
                }), plants.end());
        }
        else if (type == "month") {
            std::string op, month;
            iss >> op >> month;
            if (op == "=") {
                plants.erase(std::remove_if(plants.begin(), plants.end(),
                    [&month](const std::shared_ptr<Plant>& plant) {
                        return plant->getType() == "Shrub" &&
                            static_cast<Shrub*>(plant.get())->compare(month, "=");
                    }), plants.end());
            }
        }
        else if (type == "spine") {
            std::string op;
            double spine_length;
            iss >> op >> spine_length;
            plants.erase(std::remove_if(plants.begin(), plants.end(),
                [&op, spine_length](const std::shared_ptr<Plant>& plant) {
                    return plant->getType() == "Cactus" &&
                        plant->compare(spine_length, op);
                }), plants.end());
        }
    }
};

// Класс PlantContainer
class PlantContainer {
private:
    std::vector<std::shared_ptr<Plant>> plants;

public:
    void addPlant(std::shared_ptr<Plant> plant) {
        plants.push_back(plant);
    }

    void removePlants(const std::string& condition) {
        RemovePlantCommand removeCommand(plants, condition);
        removeCommand.execute();
    }

    void printPlants() const {
        if (plants.empty()) {
            std::cout << "No plants in the container." << std::endl;
            return;
        }

        std::cout << "Plants in the container:" << std::endl;
        for (const auto& plant : plants) {
            std::cout << plant->toString() << std::endl;
        }
    }
};

// Функция для обработки ввода пользователя
void processCommand(PlantContainer& container, const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "ADD") {
        std::string type, name;
        std::getline(iss, type, ','); // Чтение до запятой
        std::getline(iss, name, ','); // Чтение имени

        if (type == " tree") {
            int age;
            iss >> age;
            container.addPlant(std::make_shared<Tree>(name, age));
        }
        else if (type == " shrub") {
            std::string flowering_month;
            iss >> flowering_month;
            container.addPlant(std::make_shared<Shrub>(name, flowering_month));
        }
        else if (type == " cactus") {
            double spine_length;
            iss >> spine_length;
            container.addPlant(std::make_shared<Cactus>(name, spine_length));
        }
        else {
            std::cout << "Неизвестный тип растения." << std::endl;
        }
    }
    else if (cmd == "REM") {
        std::string condition;
        std::getline(iss, condition);
        condition.erase(0, condition.find_first_not_of(" \n\r\t")); // Удаление пробелов слева
        container.removePlants(condition);
    }
    else if (cmd == "PRINT") {
        container.printPlants();
    }
    else {
        std::cout << "Неизвестная команда." << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "");
    PlantContainer container;
    std::string command;

    while (true) {
        std::cout << "Введите команду (ADD, REM, PRINT или EXIT): ";
        std::getline(std::cin, command);

        if (command == "EXIT") {
            break;
        }

        processCommand(container, command);
    }

    return 0;
}
