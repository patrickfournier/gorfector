#pragma once

#include "Point.hpp"

namespace Gorfector
{
    /**
     * \brief A template structure representing a rectangle with position and size.
     *
     * \tparam T The type of the rectangle's coordinates and dimensions (e.g., int, float, double).
     */
    template<typename T>
    struct Rect
    {
        T x{}; ///< The x-coordinate of the rectangle's position.
        T y{}; ///< The y-coordinate of the rectangle's position.
        T width{}; ///< The width of the rectangle.
        T height{}; ///< The height of the rectangle.

        /**
         * \brief Calculates the minimum x-coordinate of the rectangle.
         *
         * \return The minimum x-coordinate.
         */
        T MinX() const
        {
            if (width < 0)
            {
                return x + width;
            }
            return x;
        }

        /**
         * \brief Calculates the minimum y-coordinate of the rectangle.
         *
         * \return The minimum y-coordinate.
         */
        T MinY() const
        {
            if (height < 0)
            {
                return y + height;
            }
            return y;
        }

        /**
         * \brief Calculates the maximum x-coordinate of the rectangle.
         *
         * \return The maximum x-coordinate.
         */
        T MaxX() const
        {
            if (width < 0)
            {
                return x;
            }
            return x + width;
        }

        /**
         * \brief Calculates the maximum y-coordinate of the rectangle.
         *
         * \return The maximum y-coordinate.
         */
        T MaxY() const
        {
            if (height < 0)
            {
                return y;
            }
            return y + height;
        }

        /**
         * \brief Compares two rectangles for equality.
         *
         * \param rect The rectangle to compare with.
         * \return True if both rectangles have the same position and dimensions, false otherwise.
         */
        bool operator==(const Rect &rect) const
        {
            return x == rect.x && y == rect.y && width == rect.width && height == rect.height;
        }

        /**
         * \brief Subtracts a 2D point from the rectangle's position, translating the rectangle.
         *
         * \param point The point to subtract.
         * \return A reference to this rectangle after subtraction.
         */
        Rect &operator-=(const Point<double> &point)
        {
            x -= point.x;
            y -= point.y;
            return *this;
        }

        /**
         * \brief Subtracts a 2D point from a rectangle, creating a new, translated rectangle.
         *
         * \param lhs The rectangle to subtract from.
         * \param rhs The point to subtract.
         * \return A new rectangle with the updated position.
         */
        friend Rect<double> operator-(Rect lhs, const Point<double> &rhs)
        {
            lhs -= rhs;
            return lhs;
        }
    };
}
