#include "integer.hh"

#include <algorithm>

integer::integer(long long value) {
    this->number.clear();
    this->negative = value < 0;
    // in case of value == INTMAX_MIN
    // where value = -value will overflow
    // so save partial to *this->number first
    number.push_back(std::abs(value % integer::base));
    if ((value /= integer::base) < 0) value = -value;
    while (value) {
        this->number.push_back(value % integer::base);
        value /= integer::base;
    }
}

std::istream &operator>>(std::istream &is, integer &rhs) {
    rhs.number.clear();
    rhs.negative = false;
    int next_char = is.peek();
    while (isspace(next_char)) {
        is.get();
        next_char = is.peek();
    }
    // negative or positive
    if (next_char == '-' || next_char == '+') {
        rhs.negative = next_char == '-';
        is.get();
        next_char = is.peek();
    }
    // if non-digital character
    if (!isdigit(next_char)) {
        rhs.number.push_back(0);
        is.clear(std::ios::failbit);
        return is;
    }
    while (next_char == '0') {
        // prefix zeros have no sense
        is.get();
        next_char = is.peek();
    }
    // start reading
    int32_t value = 0, length = 1;
    while (isdigit(is.peek())) {
        next_char = is.get();
        (value *= 10) += next_char - '0';
        if ((length *= 10) == integer::base) {
            rhs.number.push_back(value);
            value = 0; length = 1;
        }
    }
    if (length != 1) {
        rhs.number.push_back(value);
        const int32_t shift = integer::base / length;
        // reverse vector and adjust offset
        for (size_t i = rhs.number.size() - 1; i; --i) {
            rhs.number[i] += rhs.number[i - 1] % shift * length;
            rhs.number[i - 1] /= shift;
        }
    }
    reverse(rhs.number.begin(), rhs.number.end());
    // if all were zero
    if (!rhs.number.size()) {
        rhs.negative = false;
        rhs.number.push_back(0);
    };
    // and f**k negative zero
    return is;
}

std::ostream &operator<<(std::ostream &os, const integer &rhs) {
    if (rhs.negative) os << '-';
    os << rhs.number.back();
    // I am considering not using `auto' here
    for (auto iter = rhs.number.rbegin() + 1; iter != rhs.number.rend(); ++iter) {
        if (*iter < 100000000) os << '0';
        if (*iter < 10000000) os << '0';
        if (*iter < 1000000) os << '0';
        if (*iter < 100000) os << '0';
        if (*iter < 10000) os << '0';
        if (*iter < 1000) os << '0';
        if (*iter < 100) os << '0';
        if (*iter < 10) os << '0';
        os << *iter;
    }
    return os;
}

integer operator+(const integer &self) {
    return self;
}

integer operator-(const integer &self) {
    integer negative = self;
    // if not zero, change the sign bit.
    if (self.number.size() != 1 || self.number.front())
        negative.negative = !self.negative;
    return negative;
}

integer &operator++(integer &self) {
    if (!self.negative) {
        size_t length = self.number.size() - 1, i = 0;
        ++self.number.front();
        while (self.number[i] == integer::base && i != length)
            self.number[i] = 0, ++self.number[++i];
        if (length == i && self.number[i] == integer::base)
            self.number[length] = 0, self.number.push_back(1);
    } else self.negative = false, --self, self.negative ^= 1;
    return self;
}

integer operator++(integer &self, int) {
    integer copy = self;
    ++self;
    return copy;
}

integer &operator--(integer &self) {
    if (!self.negative) {
        if (self.number.size() != 1 || self.number.front()) {  // self != 0
            size_t length = self.number.size() - 1, i = 0;
            --self.number.front();
            while (!~self.number[i] && i != length)
                self.number[i] = integer::base - 1, --self.number[++i];
            if (self.number.back() == 0 && length) self.number.pop_back();
        } else self.number.front() = 1, self.negative = true;
    } else self.negative = 0, ++self, self.negative ^= 1;
    return self;
}

integer operator--(integer &self, int) {
    integer copy = self;
    --self;
    return copy;
}

integer operator+(integer lhs, const integer &rhs) {
    return lhs += rhs;
}

integer &operator+=(integer &lhs, const integer &rhs) {
    if (lhs.negative == rhs.negative) {
        const size_t lenl = lhs.number.size(), lenr = rhs.number.size();
        if (lhs.number.capacity() <= lenr) lhs.number.reserve(lenr + 1);
        if (lenl < lenr) lhs.number.resize(lenr);
        int32_t carry = 0;
        for (size_t i = 0; i != lenr; ++i) {
            carry += lhs.number[i] + rhs.number[i];
            // if-else is much faster than / and %
            if (carry < integer::base) lhs.number[i] = carry;
            else lhs.number[i] = carry - integer::base;
            carry = carry >= integer::base;
            // here carry should always be 0 or 1
        }
        for (size_t i = lenr; carry && i < lenl; ++i)
            if ((carry = ((lhs.number[i] += carry) == integer::base))) lhs.number[i] = 0;
        // 1st: lhs.number[i] += carry
        // 2nd: carry = (lhs.number[i] == integer::base)
        // 3rd: if (carry) lhs.number[i] = 0
        if (carry) lhs.number.push_back(1);
    } else lhs.negative ^= 1, lhs -= rhs, lhs.negative ^= 1;
    if (lhs.number.size() == 1 && !lhs.number.front()) lhs.negative = false;
    return lhs;
}

integer operator-(integer lhs, const integer &rhs) {
    return lhs -= rhs;
}

integer &operator-=(integer &lhs, const integer &rhs) {
    if (lhs.negative == rhs.negative) {
        const size_t lenl = lhs.number.size(), lenr = rhs.number.size();
        if (lenl < lenr) lhs.number.resize(lenr);
        bool carry = false;
        if ((lhs > rhs) ^ lhs.negative) {  // if abs(lhs) > abs(rhs)
            for (size_t i = 0; i != lenr; ++i) {
                lhs.number[i] -= rhs.number[i] + carry;
                if ((carry = lhs.number[i] < 0))
                    lhs.number[i] += integer::base;
            }
            for (size_t i = lenr; carry && i < lenl; ++i)
                if ((carry = !~--lhs.number[i])) lhs.number[i] = integer::base - 1;
            // 1st: --lhs.number[i]
            // 2nd: carry = lhs.number[i] == -1
            // 3rd: if (carry) lhs.number[i] = integer::base - 1
        } else {  // if (abs(lhs) < abs(rhs)), use lhs - rhs = -(rhs - lhs)
            lhs.negative ^= 1;
            for (size_t i = 0; i != lenr; ++i) {
                lhs.number[i] = rhs.number[i] - lhs.number[i] - carry;
                if ((carry = lhs.number[i] < 0)) lhs.number[i] += integer::base;
            }
        }
    } else lhs.negative ^= 1, lhs += rhs, lhs.negative ^= 1;
    lhs.trim_prefix_zeros();
    return lhs;
}

integer operator*(const integer &lhs, const integer &rhs) {
    size_t lenl = lhs.number.size(), lenr = rhs.number.size();
    integer multiple = 0;
    multiple.number.resize(lenl + lenr);
    for (size_t i = 0; i != lenl; ++i) {
        int64_t carry = 0;
        for (size_t j = 0; j != lenr; ++j) {
            carry += (int64_t) lhs.number[i] * rhs.number[j] + multiple.number[i + j];
            multiple.number[i + j] = carry % integer::base;
            carry /= integer::base;
        }
        multiple.number[i + lenr] = carry;
    }
    multiple.negative = lhs.negative ^ rhs.negative;
    multiple.trim_prefix_zeros();
    return multiple;
}

integer &operator*=(integer &lhs, const integer &rhs) {
    return lhs = lhs * rhs;
}

integer operator/(integer lhs, const int32_t &rhs) {
    return lhs /= rhs;
}

integer &operator/=(integer &lhs, const int32_t &rhs) {
    int64_t reminder = 0, den = abs(rhs);
    lhs.negative ^= rhs < 0;
    std::vector<int32_t>::const_reverse_iterator re = lhs.number.rend();
    for (auto rit = lhs.number.rbegin(); rit != re; ++rit)
        (reminder *= integer::base) += *rit, *rit = reminder / den, reminder %= den;
    lhs.trim_prefix_zeros();
    return lhs;
}

integer operator%(const integer &lhs, const int32_t &rhs) {
    int64_t rem = 0;
    std::vector<int32_t>::const_reverse_iterator re = lhs.number.rend();
    for (auto rit = lhs.number.rbegin(); rit != re; ++rit)
        ((rem *= integer::base) += *rit) %= rhs;
    if (lhs.negative) rem = -rem;  // performs like c
    return rem;
}

integer &operator%=(integer &lhs, const int32_t &rhs) {
    return lhs = lhs % rhs;
}

bool operator==(const integer &lhs, const integer &y) {
    return lhs.negative == y.negative && lhs.number == y.number;
}

bool operator!=(const integer &lhs, const integer &rhs) {
    return lhs.negative != rhs.negative || lhs.number != rhs.number;
}

bool operator<(const integer &lhs, const integer &rhs) {
    if (lhs.negative ^ rhs.negative) return lhs.negative;
    const size_t lenl = lhs.number.size(), lenr = rhs.number.size();
    if (lenl != lenr) return (lenl < lenr) ^ lhs.negative;
    for (size_t i = lenl - 1; i; --i)
        if (lhs.number[i] != rhs.number[i])
            return (lhs.number[i] < rhs.number[i]) ^ lhs.negative;
    return (lhs.number.front() < rhs.number.front()) ^ rhs.negative;
}

bool operator>(const integer &lhs, const integer &rhs) {
    if (lhs.negative ^ rhs.negative) return rhs.negative;
    const size_t lenl = lhs.number.size(), lenr = rhs.number.size();
    if (lenl != lenr) return (lenl > lenr) ^ rhs.negative;
    for (size_t i = lenr - 1; i; --i)
        if (lhs.number[i] != rhs.number[i])
            return
                    (lhs.number[i] > rhs.number[i]) ^ rhs.negative;
    return (lhs.number.front() > rhs.number.front()) ^ rhs.negative;
}

bool operator<=(const integer &lhs, const integer &rhs) {
    if (lhs.negative ^ rhs.negative) return lhs.negative;
    const size_t lenl = lhs.number.size(), lenr = rhs.number.size();
    if (lenl != lenr) return (lenl < lenr) ^ lhs.negative;
    for (int i = lenl - 1; i; --i)
        if (lhs.number[i] != rhs.number[i])
            return (lhs.number[i] < rhs.number[i]) ^ lhs.negative;
    return lhs.number.front() == rhs.number.front()
           || (lhs.number.front() < rhs.number.front()) ^ lhs.negative;
}

bool operator>=(const integer &lhs, const integer &rhs) {
    if (lhs.negative ^ rhs.negative) return rhs.negative;
    const size_t lenl = lhs.number.size(), lenr = rhs.number.size();
    if (lenl != lenr) return (lenl > lenr) ^ rhs.negative;
    for (int i = lenr - 1; i; --i)
        if (lhs.number[i] != rhs.number[i])
            return (lhs.number[i] > rhs.number[i]) ^ rhs.negative;
    return lhs.number.front() == rhs.number.front()
           || (lhs.number.front() > rhs.number.front()) ^ rhs.negative;
}

integer::operator bool() {
    return this->negative || this->number.size() != 1 || this->number.front();
}

void integer::trim_prefix_zeros() {
    size_t length = this->number.size() - 1;
    while (!this->number[length] && length) --length;
    this->number.resize(length + 1);
    // Take care of negative zero
    if (!length && !this->number.back()) this->negative = false;
}

