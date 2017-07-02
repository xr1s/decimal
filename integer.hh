#include <iostream>
#include <vector>

#ifndef _INTEGER_H_
# define _INTEGER_H_

class integer {
public:
    integer(long long x = 0);
    explicit operator bool();

    friend std::istream &operator>>(std::istream&, integer&);
    friend std::ostream &operator<<(std::ostream&, const integer&);

    friend integer operator-(const integer&);
    friend integer operator+(const integer&);

    friend integer &operator++(integer&);
    friend integer operator++(integer&, int);
    friend integer &operator--(integer&);
    friend integer operator--(integer&, int);

    friend integer operator+(integer, const integer&);
    friend integer operator-(integer, const integer&);
    // TODO: support Fast Fourier Transform
    friend integer operator*(const integer&, const integer&);
    friend integer operator/(const integer&, const integer&);
    friend integer operator/(integer, const int32_t&);
    friend integer operator%(const integer&, const integer&);
    friend integer operator%(const integer&, const int32_t&);

    friend integer &operator+=(integer&, const integer&);
    friend integer &operator-=(integer&, const integer&);
    friend integer &operator*=(integer&, const integer&);
    friend integer &operator/=(integer&, const integer&);
    friend integer &operator/=(integer&, const int32_t&);
    friend integer &operator%=(integer&, const integer&);
    friend integer &operator%=(integer&, const int32_t&);

    friend bool operator==(const integer&, const integer&);
    friend bool operator!=(const integer&, const integer&);
    friend bool operator<(const integer&, const integer&);
    friend bool operator>(const integer&, const integer&);
    friend bool operator<=(const integer&, const integer&);
    friend bool operator>=(const integer&, const integer&);

private:
    constexpr static int32_t base = 1000000000;
    std::vector<int32_t> number; bool negative;

    void trim_prefix_zeros();
};

#endif // _INTEGER_H_
