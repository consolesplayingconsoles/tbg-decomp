<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_course_0()
    {
        $this->testCourse(
            course: 0,
            days: 127,
            exp: 1_000_000,
            writes: [-1]
        );
    }

    public function test_course_1()
    {
        $this->testCourse(
            course: 1, days: 8, exp: 4000, writes: [1, -1],
        );
    }

    public function test_course_1_day_check()
    {
        $this->testCourse(
            course: 1, days: 3, exp: 4000, writes: [-1],
        );
    }

    public function test_course_1_exp_check()
    {
        $this->testCourse(
            course: 1, days: 8, exp: 3999, writes: [-1],
        );
    }

    public function test_course_2()
    {
        $this->testCourse(
            course: 2, days: 9, exp: 5500, writes: [2, -1],
        );
    }

    public function test_course_2_day_check()
    {
        $this->testCourse(
            course: 2, days: 8, exp: 5500, writes: [-1],
        );
    }

    public function test_course_2_exp_check()
    {
        $this->testCourse(
            course: 2, days: 9, exp: 5499, writes: [-1],
        );
    }

    public function test_course_3()
    {
        $this->testCourse(
            course: 3, days: 5, exp: 2000, writes: [3, -1],
        );
    }

    public function test_course_3_day_check()
    {
        $this->testCourse(
            course: 3, days: 4, exp: 2000, writes: [-1],
        );
    }

    public function test_course_3_exp_check()
    {
        $this->testCourse(
            course: 3, days: 5, exp: 1999, writes: [-1],
        );
    }

    public function test_course_4()
    {
        $this->testCourse(
            course: 4, days: 11, exp: 8000, writes: [4, -1],
        );
    }

    public function test_course_4_day_check()
    {
        $this->testCourse(
            course: 4, days: 10, exp: 8000, writes: [-1],
        );
    }

    public function test_course_4_exp_check()
    {
        $this->testCourse(
            course: 4, days: 11, exp: 7999, writes: [-1],
        );
    }

    public function test_course_5()
    {
        $this->testCourse(
            course: 5, days: 13, exp: 12000, writes: [5, -1],
        );
    }

    public function test_course_5_day_check()
    {
        $this->testCourse(
            course: 5, days: 12, exp: 12000, writes: [-1],
        );
    }

    public function test_course_5_exp_check()
    {
        $this->testCourse(
            course: 5, days: 13, exp: 11999, writes: [-1],
        );
    }

    public function test_course_6()
    {
        $this->testCourse(
            course: 6, days: 127, exp: 1_000_000, writes: [-1],
        );
    }

    public function test_course_7()
    {
        $this->testCourse(
            course: 7, days: 3, exp: 500, writes: [7, -1],
        );
    }

    public function test_course_7_day_check()
    {
        $this->testCourse(
            course: 7, days: 2, exp: 500, writes: [-1],
        );
    }

    public function test_course_7_exp_check()
    {
        $this->testCourse(
            course: 7, days: 3, exp: 499, writes: [-1],
        );
    }

    public function test_course_8()
    {
        $this->testCourse(
            course: 8, days: 6, exp: 3000, writes: [8, -1],
        );
    }

    public function test_course_8_day_check()
    {
        $this->testCourse(
            course: 8, days: 5, exp: 3000, writes: [-1],
        );
    }

    public function test_course_8_exp_check()
    {
        $this->testCourse(
            course: 8, days: 6, exp: 2999, writes: [-1],
        );
    }

    public function test_all_courses()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseUnlockedFlags([0, 0, 0, 0, 0, 0, 0, 0, 0]);
        // days_0x00
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 127);
        // exp_0x90
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x90, 1_000_000);

        // -- Act ----------------------
        $this->call('_CourseMenuBuildCourseUnlockList_8c0172dc');

        // -- Assert -------------------
        foreach ([1, 2, 3, 4, 5, 7, 8, -1] as $offset => $value) {
            $this->shouldWriteByte($this->addressOf('_var_coursesToUnlock_8c225fd4') + $offset, $value);
        }
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_coursesToUnlock_8c225fd4', 4 * 9);
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
    }

    private function initCourseUnlockedFlags(array $values)
    {
        if (count($values) !== 9) {
            throw new \InvalidArgumentException('Expected exactly 9 values for _var_progress_8c1ba1cc.');
        }

        foreach ($values as $index => $value) {
            $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0x44 + $index * 8, $value);
        }
    }

    private function testCourse(
        int $course,
        int $days,
        int $exp,
        array $writes
    ) {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseUnlockedFlags([
            0,
            $course === 1 ? 0 : 1,
            $course === 2 ? 0 : 1,
            $course === 3 ? 0 : 1,
            $course === 4 ? 0 : 1,
            $course === 5 ? 0 : 1,
            0,
            $course === 7 ? 0 : 1,
            $course === 8 ? 0 : 1
        ]);
        // days_0x00
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), $days);
        // exp_0x90
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0x90, $exp);

        // -- Act ----------------------
        $this->call('_CourseMenuBuildCourseUnlockList_8c0172dc');

        // -- Assert -------------------
        foreach ($writes as $offset => $value) {
            $this->shouldWriteByte($this->addressOf('_var_coursesToUnlock_8c225fd4') + $offset, $value);
        }

        $this->shouldReturn(count($writes) - 1);
    }
};
