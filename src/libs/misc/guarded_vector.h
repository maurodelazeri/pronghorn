//
// Created by mauro on 5/23/19.
//

#pragma once

#include <vector>
#include <mutex>

/*
	Implement the thread safe version template std::vector.
	I implemented methods which only i needed.
*/

template<class T>
class guarded_vector
{
public:
    guarded_vector(int size = 0)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.resize(size);
    };

    T get_value(int i) const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector[i];
    };

    int size() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.size();
    };

    void resize(int size)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.resize(size);
    };

    void erase(int position)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.erase(std::remove(vector.begin(), vector.end(), position), vector.end());
    };

    void erase_range(int max)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.erase(vector.begin(),vector.begin()+max);
    };

    void push_back(const T& elem)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.push_back(elem);
    };

    void set_value(int i, const T& value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector[i] = value;
    };

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.clear();
    };

private:
    std::vector<T>			vector;
    mutable std::mutex		mutex;
};
