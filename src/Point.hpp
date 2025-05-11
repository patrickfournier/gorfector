#pragma once

namespace Gorfector
{
    /**
     * \brief A template structure representing a 2D point with x and y coordinates.
     *
     * \tparam T The type of the coordinates (e.g., int, float, double).
     */
    template<typename T>
    struct Point
    {
        T x{}; ///< The x-coordinate of the point.
        T y{}; ///< The y-coordinate of the point.

        /**
         * \brief Compares two points for equality.
         *
         * \param point The point to compare with.
         * \return True if both points have the same x and y coordinates, false otherwise.
         */
        bool operator==(const Point &point) const
        {
            return x == point.x && y == point.y;
        }

        /**
         * \brief Adds the coordinates of another point to this point.
         *
         * \param point The point to add.
         * \return A reference to this point after addition.
         */
        Point &operator+=(const Point &point)
        {
            x += point.x;
            y += point.y;
            return *this;
        }

        /**
         * \brief Adds two points together.
         *
         * \param lhs The left-hand side point.
         * \param rhs The right-hand side point.
         * \return A new point that is the sum of the two points.
         */
        friend Point operator+(Point lhs, const Point &rhs)
        {
            lhs += rhs;
            return lhs;
        }
    };
}
