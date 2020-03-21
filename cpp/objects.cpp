/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Idle.

    Idle is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Idle is distributed in the hope that it will be fun and useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Idle. If not, see <http://www.gnu.org/licenses/>.
*/

#include <type_traits>
#include <variant>
#include "gl.hpp"
#include "objects.hpp"
#include "drawable.hpp"
#include "draw_text.hpp"

// void idle::Player::step(::overlay & parent)
// {
//  if (possesed)
//  {
//         if (parent.pointer.pressed) {
//             const float dir = atan2f(parent.height * .5462f - parent.pointer.pos.y, -(parent.width * .5f - parent.pointer.pos.x));
//             possesed->AddMotion(dir, .25f + .1f * fabsf(cosf(dir)));
//         }
//  }
// }

// void idle::object::add_motion(float rad, float v) {
//     if(fabsf(speed) >= 0.0f)
//     {
//         float x = cosf(direction) * speed + cosf(rad) * v;
//         float y = sinf(direction) * speed + sinf(rad) * v;
//         direction = atan2f(y, x);
//         speed = sqrtf(y * y + x * x);
//     }
//     else
//     {
//         direction = rad;
//         if(direction > F_PI)
//             direction -= F_TAU;
//         speed = v;
//     }
// }




namespace idle
{
// void object::step(collection& top)
// {
//     std::visit([this, &top](auto& var)
//     {
//         using T = std::remove_cv_t<std::remove_reference_t<decltype(var)>>;
//         if constexpr (std::is_same_v<T, gui_elem>)
//         {
//             if (var.trigger == gui_elem::trigger_t::SelectBone)
//             {
//                 pos += (*reinterpret_cast<const point_t*>(var.data.f + 2) - pos) / 16;
//             }
//         }
//     }, data);
// }
//
// void object::draw(const graphics::core& gl) const
// {
//     std::visit([this, &gl](auto& var)
//     {
//         using T = std::remove_cv_t<std::remove_reference_t<decltype(var)>>;
//         if constexpr (std::is_same_v<T, gui_elem>)
//         {
//             if (var.draw_proc)
//             {
//                 var.draw_proc(gl, pos, var);
//                 return;
//             }
//             if (var.bg.a > 0.005f)
//             {
//                 gl.pfill.use();
//                 gl.pfill.set_color(var.bg, var.hover ? var.bg.a : var.bg.a / 2);
//                 gl.pfill.set_transform(mat4x4_t::translate(pos));
//                 fill_rectangle(gl.pfill, var.bound);
//             }
//             // parent.gl.ptext.use();
//             // parent.gl.ptext.set_color(1,1,1,1);
//             // draw_text(var.hover ? "hovered" : "--", pos, 12, TextAlign::Near, TextAlign::Near);
//         }
//     }, data);
// }

void object::_apply_physics(){
//     pos.x += cosf(direction) * speed;
//     pos.y -= sinf(direction) * speed;
//     if(friction > 0) {
//         if(fabsf(speed) < .015f)
//             speed = 0;
//         else
//             speed *= (1.0f - std::min(friction, .9998f));
//     }
}

object& collection::create_object(float x, float y)
{
    return create_object({x, y});
}

object& collection::create_object(idle::point_t pos)
{
    object & o = data.emplace();
    object ** p = &first_node;
    o.pos = pos;
    while (*p != nullptr)
        p = &(*p)->next_obj_ptr;
    *p = &o;
    return o;
}

void collection::destroy_all_objects()
{
    data.clear();
    first_node = nullptr;
}

void collection::sort_objects_by_depth()
{
    std::lock_guard<std::mutex> lock(node_access);
    if (auto p = &first_node; *p != nullptr) {
        if (!replacements.empty())
        {
            while (!!*p)
            {
                if (auto node = replacements.extract(*p))
                    *p = node.mapped();
                else
                    p = &(*p)->next_obj_ptr;
            }
            p = &first_node;
            if (!*p)
                return;
        }
        float d1 = (*p)->depth.load(std::memory_order_relaxed);
        while (!!(*p)->next_obj_ptr)
        {
            idle::object * const n = *p;
            const float d2 = n->next_obj_ptr->depth.load(std::memory_order_relaxed);
            if (d2 < d1) {
                *p = n->next_obj_ptr;
                p = &n->next_obj_ptr->next_obj_ptr;
                n->next_obj_ptr = n->next_obj_ptr->next_obj_ptr;
                *p = n;
            }
            else
            {
                d1 = d2;
                p = &n->next_obj_ptr;
            }
        }
    }
}

void collection::iterate_object_step()
{
    std::lock_guard<std::mutex> lock(gen_access);
    for (auto o = data.begin(); o != data.end();)
    {
        if (o->is_active)
        {
            o->step(*this);
        }
        if (o->destroy_flag)
        {
            std::lock_guard<std::mutex> lock(node_access);
            replacements.try_emplace(&*o, o->next_obj_ptr);
            data.erase(o);
        }
        else ++o;
    }
}

void collection::iterate_object_draw(const graphics::core& gl)
{
    std::scoped_lock lock1(node_access, gen_access);
    for (auto o = first_node; o; o = o->next_obj_ptr)
    {
        if (!o->is_hidden)
            o->draw(gl);
    }
}

}  // namespace idle










#if FALSE

//idle::object * idle::Control::CheckCollision(const idle::object *q)
//{
//    static const auto calcp = [](double _X, double _Y) { return static_cast<FLOAT>(sqrt(_X * _X + _Y * _Y)); };
//        for(GAME::object * o = game_first_object; o != nullptr; o = o->next_obj_ptr)
//            if((o->mSolid) && (o!=q) /*&& (o->mTeam!=q->mTeam)*/ && (calcp(o->mX - q->mX, o->mY - q->mY) <= (o->mCX+o->mCY+q->mCX+q->mCY)/2.0f))
//                return o;
//        return nullptr;
//}

// void idle::Entity::step()
// {
//  ApplyMotion();
//     if (_swr != nullptr) {
//         if (mAnimationRepeat) {
//             mAnimation += mAnimationSpeed;
//             if (mAnimation > 1.f)
//                 mAnimation -= 1.f;
//         }
//         else {
//             if (mAnimation < 1.f)
//                 mAnimation =
//                         mAnimation + mAnimationSpeed < 1.f ? mAnimation + mAnimationSpeed : 1.f;
//         }
//     }
//  if (mImmunity>0)
//      --mImmunity;
//  if (mSpeed > 0) {
//         mFacing = static_cast<uint8_t>(
//                 ((383
//                   - static_cast<int>(mDir * 180.f * F_1_PI)
//                  ) / 45 + 2) % 8
//         ); // 360 + 45 / 2 => F_TAU + F_PI_4
//     }
// }

// void idle::Entity::SetAnimation(uint16_t id)
// {
//     if (_swr != nullptr) {
//         if (id >= _swr->size())
//             id = 0;
//         mAniID = id;
//         mAnimation = 0.f;
//      if ((mAnimationRepeat = (*_swr)[id].repeat))
//          mAnimationSpeed = (*_swr)[id].speed / (APPLICATION_FPS * (*_swr)[id].f.size());
//      else
//          mAnimationSpeed = (*_swr)[id].speed / (APPLICATION_FPS * ((*_swr)[id].f.size() - 1));
//     }
// }

void idle::Humanoid::step()
{
    idle::Entity::step();
    const float asp = fabsf(mSpeed);
    switch (mState) {
        case 0:
            if (mAniID == 0 && asp > 0.168f)
            {
                SetAnimation(1);
            }
            else if (mAniID == 1)
            {
                if (asp < .168f) {
                    SetAnimation(0);
                    mSpeed = 0;
                }
                else if (asp > 3.2f)
                    SetAnimation(2);
            }
            else if (mAniID == 2 && asp < 3.2f)
            {
                SetAnimation(1);
            }
            break;
    }
}

void idle::Humanoid::draw() const
{
#define HUMANOID_PREPARE    const uint8_t facing = static_cast<uint8_t>(abs(mFacing - FACEDIR_DOWN));\
    const sprite_t &head = Database::instance->sprites[mSprite[0] + facing],\
            &body = Database::instance->sprites[mSprite[1] + facing],\
            *legu = &Database::instance->sprites[mSprite[2] + facing * 2],\
            *legl = &Database::instance->sprites[mSprite[3] + facing * 2],\
            *armu = &Database::instance->sprites[mSprite[4] + facing * 2],\
            *arml = armu + 10;\
    \
    const bool switch_sides = !((mFacing >= FACEDIR_RIGHT_SIDE) && (mFacing <= FACEDIR_LEFT_SIDE));\
    const uint8_t face = FACEDIR_DOWN - abs(mFacing - FACEDIR_DOWN);\
    const float arm_y_dipl = face % 2 ? (switch_sides ? _jd->m[15] : -_jd->m[15]) : 0;\
    \
    const auto &aID = (*_swr)[mAniID >= _swr->size() ? 0 : mAniID].f;\
    float t = fabsf(mAnimation) * (aID.size() - !mAnimationRepeat);\
    const unsigned int a = static_cast<unsigned int>(t) % aID.size();\
    t -= a;\
    const float * ani = aID[a].m[face], * anip1 = aID[(a + 1) % aID.size()].m[face];\
    \
    mat4x4_t mat_bdy = mat4x4_t::rotate(mFacing > FACEDIR_DOWN ? -ani[byr] - (anip1[byr] - ani[byr]) * t : ani[byr] + (anip1[byr] - ani[byr]) * t)\
                                % mat4x4_t::scale(1, 1 + ani[bys] + (anip1[bys] - ani[bys]) * t)\
                                % mat4x4_t::translate(0, -_jd->m[0] + ani[byy] + (anip1[byy] - ani[byy]) * t)\
                                % mat4x4_t::scale(mDrawScale) % mat4x4_t::translate(mX, mY) % parent.mCamera,\
            mat_waist = mat4x4_t::translate(0, _jd->m[2]) % mat_bdy,\
            mat_knee = mat4x4_t::translate(0, _jd->m[4]),\
            mat_shoulder = mat_bdy,\
            mat_elbow = mat4x4_t::translate(0, _jd->m[3]);\
    if (mFacing > FACEDIR_DOWN)\
    {\
        mat_bdy.FMMultiply_2Doptimal(mat4x4_t::scale(-1.f, 1.f));\
        mat_waist.FMMultiply_2Doptimal(mat4x4_t::scale(-1.f, 1.f));\
        mat_shoulder.FMMultiply_2Doptimal(mat4x4_t::scale(-1.f, 1.f));\
    }\
    const mat4x4_t dis_xr = mat4x4_t::translate(_jd->m[face + 5], 0.f) % mat_waist,\
            dis_xl = mat4x4_t::translate(-_jd->m[face + 5], 0.f) % mat_waist,\
            dis_xar = mat4x4_t::translate(_jd->m[face + 10], arm_y_dipl) % mat_shoulder,\
            dis_xal = mat4x4_t::translate(-_jd->m[face + 10], -arm_y_dipl) % mat_shoulder;\
    mat4x4_t mat_head = mat4x4_t::rotate(mFacing > FACEDIR_DOWN ? -ani[hda] - (anip1[hda] - ani[hda]) * t : ani[hda] + (anip1[hda] - ani[hda]) * t) % mat4x4_t::translate(0.f, -_jd->m[1]) % mat_bdy;\
    mat4x4_t mat_right_leg_upper, mat_right_leg_bottom, mat_right_leg_scale, mat_left_leg_upper, mat_left_leg_bottom, mat_left_leg_scale, mat_right_arm_upper, mat_right_arm_bottom, mat_left_arm_upper, mat_left_arm_bottom, mat_right_arm_scale, mat_left_arm_scale;\

    HUMANOID_PREPARE;

    if ((mFacing == FACEDIR_DOWN) || (mFacing == FACEDIR_UP))
    {
        const float fls = ani[sll] + (anip1[sll] - ani[sll]) * t,
                frs = ani[srl] + (anip1[srl] - ani[srl]) * t;
        const bool invert_order = fls > frs;
        mat_left_leg_scale = mat4x4_t::scale(1.f, 1.f + fls);
        mat_left_leg_upper = mat4x4_t::rotate(ani[ull] + (anip1[ull] - ani[ull]) * t);
        mat_left_leg_bottom = mat4x4_t::scale(1.f, 1.f + (ani[lll] + (anip1[lll] - ani[lll]) * t) / 10.f);

        mat_right_leg_scale = mat4x4_t::scale(1.f, 1.f + frs);
        mat_right_leg_upper = mat4x4_t::rotate(ani[url] + (anip1[url] - ani[url]) * t);
        mat_right_leg_bottom = mat4x4_t::scale(1.f, 1.f + (ani[lrl] + (anip1[lrl] - ani[lrl]) * t) / 10.f);

        mat_left_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sla] + (anip1[sla] - ani[sla]) * t);
        mat_left_arm_upper = mat4x4_t::rotate(ani[ula] + (anip1[ula] - ani[ula]) * t);
        mat_left_arm_bottom = mat4x4_t::rotate(ani[lla] + (anip1[lla] - ani[lla]) * t);
        mat_right_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sra] + (anip1[sra] - ani[sra]) * t);
        mat_right_arm_upper = mat4x4_t::rotate(ani[ura] + (anip1[ura] - ani[ura]) * t);
        mat_right_arm_bottom = mat4x4_t::rotate(ani[lra] + (anip1[lra] - ani[lra]) * t);

        if (switch_sides)
        {
            mat_left_leg_upper %= mat_left_leg_scale % dis_xl;
            mat_right_leg_upper %= mat_right_leg_scale % dis_xr;
            mat_left_arm_upper %= mat_left_arm_scale % dis_xal;
            mat_right_arm_upper %= mat_right_arm_scale % dis_xar;
        }
        else
        {
            mat_left_leg_upper %= mat_left_leg_scale % dis_xr;
            mat_right_leg_upper %= mat_right_leg_scale % dis_xl;
            mat_left_arm_upper %= mat_left_arm_scale % dis_xar;
            mat_right_arm_upper %= mat_right_arm_scale % dis_xal;
        }
        if (mFacing == FACEDIR_UP)
        {
            set_transform(mat_right_arm_bottom % mat_elbow % mat_right_arm_upper);
            draw_sprite(*arml);
            set_transform(mat_left_arm_bottom % mat_elbow % mat_left_arm_upper);
            draw_sprite(*(arml + 1));
        }
        if (!invert_order)
        {
            set_transform(mat_left_leg_upper);
            draw_sprite(*(legu + 1));
            set_transform(mat_left_leg_bottom % mat_knee % mat_left_leg_upper);
            draw_sprite(*(legl + 1));
        }
        set_transform(mat_right_leg_upper);
        draw_sprite(*legu);
        set_transform(mat_right_leg_bottom % mat_knee % mat_right_leg_upper);
        draw_sprite(*legl);
        if (invert_order)
        {
            set_transform(mat_left_leg_upper);
            draw_sprite(*(legu + 1));
            set_transform(mat_left_leg_bottom % mat_knee % mat_left_leg_upper);
            draw_sprite(*(legl + 1));
        }

        set_transform(mat_right_arm_upper);
        draw_sprite(*armu);
        set_transform(mat_left_arm_upper);
        draw_sprite(*(armu + 1));

        set_transform(mat_bdy);
        draw_sprite(body);
        set_transform(mat_head);
        draw_sprite(head); //, point_t(0.f, body.bound.top - head.bound.bottom - _jd->m[15]));

        if (mFacing == FACEDIR_DOWN)
        {
            set_transform(mat_right_arm_bottom % mat_elbow % mat_right_arm_upper);
            draw_sprite(*arml);
            set_transform(mat_left_arm_bottom % mat_elbow % mat_left_arm_upper);
            draw_sprite(*(arml + 1));
        }
    }
    else
    {
        mat_left_leg_scale = mat4x4_t::scale(1.f, 1.f + ani[sll] + (anip1[sll] - ani[sll])*t);
        mat_left_leg_upper = mat4x4_t::rotate(ani[ull] + (anip1[ull] - ani[ull]) * t);
        mat_left_leg_bottom = mat4x4_t::rotate(ani[lll] + (anip1[lll] - ani[lll]) * t);

        mat_right_leg_scale = mat4x4_t::scale(1.f, 1.f + ani[srl] + (anip1[srl] - ani[srl])*t);
        mat_right_leg_upper = mat4x4_t::rotate(ani[url] + (anip1[url] - ani[url]) * t);
        mat_right_leg_bottom = mat4x4_t::rotate(ani[lrl] + (anip1[lrl] - ani[lrl]) * t);

        mat_left_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sla] + (anip1[sla] - ani[sla])*t);
        mat_left_arm_upper = mat4x4_t::rotate(ani[ula] + (anip1[ula] - ani[ula]) * t);
        mat_left_arm_bottom = mat4x4_t::rotate(ani[lla] + (anip1[lla] - ani[lla]) * t);
        mat_right_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sra] + (anip1[sra] - ani[sra])*t);
        mat_right_arm_upper = mat4x4_t::rotate(ani[ura] + (anip1[ura] - ani[ura]) * t);
        mat_right_arm_bottom = mat4x4_t::rotate(ani[lra] + (anip1[lra] - ani[lra]) * t);

        auto armu_r = armu, arml_r = arml, legu_r = legu, legl_r = legl;

        if (switch_sides)
        {
            mat_left_leg_upper %= mat_left_leg_scale % dis_xl;
            mat_right_leg_upper %= mat_right_leg_scale % dis_xr;
            mat_left_arm_upper %= mat_left_arm_scale % dis_xal;
            mat_right_arm_upper %= mat_right_arm_scale % dis_xar;
            ++arml_r; ++armu_r; ++legl_r; ++legu_r;
        }
        else
        {
            mat_left_leg_upper %= mat_left_leg_scale % dis_xr;
            mat_right_leg_upper %= mat_right_leg_scale % dis_xl;
            mat_left_arm_upper %= mat_left_arm_scale % dis_xar;
            mat_right_arm_upper %= mat_right_arm_scale % dis_xal;
            ++arml; ++armu; ++legl; ++legu;
        }
        set_transform(mat_left_arm_upper);
        draw_sprite(*armu);
        set_transform(mat_left_arm_bottom % mat_elbow % mat_left_arm_upper);
        draw_sprite(*arml);

        set_transform(mat_left_leg_upper);
        draw_sprite(*legu);
        set_transform(mat_left_leg_bottom % mat_knee % mat_left_leg_upper);
        draw_sprite(*legl);
        set_transform(mat_right_leg_upper);
        draw_sprite(*legu_r);
        set_transform(mat_right_leg_bottom % mat_knee % mat_right_leg_upper);
        draw_sprite(*legl_r);

        set_transform(mat_bdy);
        draw_sprite(body);
        set_transform(mat_head);
        draw_sprite(head);

        set_transform(mat_right_arm_upper);
        draw_sprite(*armu_r);
        set_transform(mat_right_arm_bottom % mat_elbow % mat_right_arm_upper);
        draw_sprite(*arml_r);
    }
    SetIdentity();
    /***********/
}

#ifdef DEBUG
inline bool get_vector_intersect(point_t p, const mat4x4_t & transformation) {
    point_t rv[4]{ { rect.left, rect.top }, { rect.right, rect.top }, { rect.right, rect.bottom }, { rect.left, rect.bottom } };
    const auto mat = transformation % Database::instance->projectionMatrix % mat4x4_t::translate(1, -1) % mat4x4_t::scale(.5f, -.5f);
    for (auto &it : rv)
        it *= mat;
    return IsInside(rv, p);
}
inline bool get_vector_intersect(const sprite_t &spr, point_t p, const mat4x4_t & transformation) {
    point_t rv[4]{ { -spr.center.x, -spr.center.y }, { spr.size.width - spr.center.x, -spr.center.y }, { spr.size.width - spr.center.x, spr.size.height - spr.center.y }, { -spr.center.x, spr.size.height - spr.center.y } };
    const auto mat = transformation % Database::instance->projectionMatrix % mat4x4_t::translate(1, -1) % mat4x4_t::scale(.5f, -.5f);
    for (auto &it : rv)
        it *= mat;
    return IsInside(rv, p);
}

idle::Humanoid::__DBP_enum idle::Humanoid::GetDrawnBodyPartDrawDebug(point_t pnt) const {
    HUMANOID_PREPARE;

    if ((mFacing == FACEDIR_DOWN) || (mFacing == FACEDIR_UP)) {
        const float fls = ani[sll] + (anip1[sll] - ani[sll]) * t,
                frs = ani[srl] + (anip1[srl] - ani[srl]) * t;
        mat_left_leg_scale = mat4x4_t::scale(1.f, 1.f + fls);
        mat_left_leg_upper = mat4x4_t::rotate(ani[ull] + (anip1[ull] - ani[ull]) * t);
        mat_left_leg_bottom = mat4x4_t::scale(1.f, 1.f + (ani[lll] + (anip1[lll] - ani[lll]) * t) / 10.f);

        mat_right_leg_scale = mat4x4_t::scale(1.f, 1.f + frs);
        mat_right_leg_upper = mat4x4_t::rotate(ani[url] + (anip1[url] - ani[url]) * t);
        mat_right_leg_bottom = mat4x4_t::scale(1.f, 1.f + (ani[lrl] + (anip1[lrl] - ani[lrl]) * t) / 10.f);

        mat_left_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sla] + (anip1[sla] - ani[sla]) * t);
        mat_left_arm_upper = mat4x4_t::rotate(ani[ula] + (anip1[ula] - ani[ula]) * t);
        mat_left_arm_bottom = mat4x4_t::rotate(ani[lla] + (anip1[lla] - ani[lla]) * t);
        mat_right_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sra] + (anip1[sra] - ani[sra]) * t);
        mat_right_arm_upper = mat4x4_t::rotate(ani[ura] + (anip1[ura] - ani[ura]) * t);
        mat_right_arm_bottom = mat4x4_t::rotate(ani[lra] + (anip1[lra] - ani[lra]) * t);

        if (switch_sides)
        {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xl;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xr;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xal;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xar;
        }
        else
        {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xr;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xl;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xar;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xal;
        }
        if (get_vector_intersect(*arml, pnt, mat4x4_t::translate(0, _jd->m[18]) * mat_right_arm_bottom * mat_elbow * mat_right_arm_upper))
            return LowerRightArm;
        if (get_vector_intersect(*arml, pnt, mat4x4_t::translate(0, _jd->m[18]) * mat_left_arm_bottom * mat_elbow * mat_left_arm_upper))
            return LowerLeftArm;
        if (get_vector_intersect(*legu, pnt, mat_left_leg_upper))
            return UpperLeftLeg;
        if (get_vector_intersect(*legl, pnt, mat4x4_t::translate(0, _jd->m[16]) * mat_left_leg_bottom * mat_knee * mat_left_leg_upper))
            return LowerLeftLeg;
        if (get_vector_intersect(*legu, pnt, mat_right_leg_upper))
            return UpperRightLeg;
        if (get_vector_intersect(*legl, pnt, mat4x4_t::translate(0, _jd->m[16]) * mat_right_leg_bottom * mat_knee * mat_right_leg_upper))
            return LowerRightLeg;

        if (get_vector_intersect(*armu, pnt, mat_right_arm_upper))
            return UpperRightArm;
        if (get_vector_intersect(*armu, pnt, mat_left_arm_upper))
            return UpperLeftArm;

        if (get_vector_intersect(body, pnt, mat_bdy))
            return Body;
        if (get_vector_intersect(head, pnt, mat_head))
            return Head;

    }
    else {
        mat_left_leg_scale = mat4x4_t::scale(1.f, 1.f + ani[sll] + (anip1[sll] - ani[sll])*t);
        mat_left_leg_upper = mat4x4_t::rotate(ani[ull] + (anip1[ull] - ani[ull]) * t);
        mat_left_leg_bottom = mat4x4_t::rotate(ani[lll] + (anip1[lll] - ani[lll]) * t);

        mat_right_leg_scale = mat4x4_t::scale(1.f, 1.f + ani[srl] + (anip1[srl] - ani[srl])*t);
        mat_right_leg_upper = mat4x4_t::rotate(ani[url] + (anip1[url] - ani[url]) * t);
        mat_right_leg_bottom = mat4x4_t::rotate(ani[lrl] + (anip1[lrl] - ani[lrl]) * t);

        mat_left_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sla] + (anip1[sla] - ani[sla])*t);
        mat_left_arm_upper = mat4x4_t::rotate(ani[ula] + (anip1[ula] - ani[ula]) * t);
        mat_left_arm_bottom = mat4x4_t::rotate(ani[lla] + (anip1[lla] - ani[lla]) * t);
        mat_right_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sra] + (anip1[sra] - ani[sra])*t);
        mat_right_arm_upper = mat4x4_t::rotate(ani[ura] + (anip1[ura] - ani[ura]) * t);
        mat_right_arm_bottom = mat4x4_t::rotate(ani[lra] + (anip1[lra] - ani[lra]) * t);

        if (switch_sides) {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xl;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xr;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xal;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xar;
        }
        else {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xr;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xl;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xar;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xal;
        }
        if (get_vector_intersect(*armu, pnt, mat_left_arm_upper))
            return UpperLeftArm;
        if (get_vector_intersect(*arml, pnt, mat_left_arm_bottom * mat_elbow * mat_left_arm_upper))
            return LowerLeftArm;

        if (get_vector_intersect(*legu, pnt, mat_left_leg_upper))
            return UpperLeftLeg;
        if (get_vector_intersect(*legl, pnt, mat_left_leg_bottom * mat_knee * mat_left_leg_upper))
            return LowerLeftLeg;
        if (get_vector_intersect(*legu, pnt, mat_right_leg_upper))
            return UpperRightLeg;
        if (get_vector_intersect(*legl, pnt, mat_right_leg_bottom * mat_knee * mat_right_leg_upper))
            return LowerRightLeg;

        if (get_vector_intersect(*armu, pnt, mat_right_arm_upper))
            return UpperRightArm;
        if (get_vector_intersect(*arml, pnt, mat_right_arm_bottom * mat_elbow * mat_right_arm_upper))
            return LowerRightArm;

        if (get_vector_intersect(body, pnt, mat_bdy))
            return Body;
        if (get_vector_intersect(head, pnt, mat_head))
            return Head;
    }
    return __DBP_enum ::None;
}

inline void _draw_rect_in_n_out_(const color_t &outline) {
    Database::instance->SetColor(in);
    fill_rectangle(rect);
    Database::instance->SetColor(outline);
    draw_rectangle(rect);
};

inline void _draw_rect_in_n_out_(const sprite_t &spr, const color_t &outline) {
    const rect_t rect { -spr.center.x, -spr.center.y, spr.size.width - spr.center.x, spr.size.height - spr.center.y };
    Database::instance->SetColor(in);
    fill_rectangle(rect);
    Database::instance->SetColor(outline);
    draw_rectangle(rect);
};

float idle::Humanoid::DrawDebug(const color_t &hl, const idle::Humanoid::Debug_body_parts &dbp) const {
    HUMANOID_PREPARE;

    set_transform(
            mat4x4_t::rotate(45_deg) % mat4x4_t::scale(1, .4f) % mat4x4_t::translate(mX, mY) % parent.mCamera);
    fill_rectangle(rect_t(-1, -20, 1, 20));
    fill_rectangle(rect_t(-20, -1, 20, 1));

    if ((mFacing == FACEDIR_DOWN) || (mFacing == FACEDIR_UP))
    {
        const float fls = ani[sll] + (anip1[sll] - ani[sll]) * t,
                frs = ani[srl] + (anip1[srl] - ani[srl]) * t;
        const bool invert_order = fls > frs;
        mat_left_leg_scale = mat4x4_t::scale(1.f, 1.f + fls);
        mat_left_leg_upper = mat4x4_t::rotate(ani[ull] + (anip1[ull] - ani[ull]) * t);
        mat_left_leg_bottom = mat4x4_t::scale(1.f, 1.f + (ani[lll] + (anip1[lll] - ani[lll]) * t) / 10.f);

        mat_right_leg_scale = mat4x4_t::scale(1.f, 1.f + frs);
        mat_right_leg_upper = mat4x4_t::rotate(ani[url] + (anip1[url] - ani[url]) * t);
        mat_right_leg_bottom = mat4x4_t::scale(1.f, 1.f + (ani[lrl] + (anip1[lrl] - ani[lrl]) * t) / 10.f);

        mat_left_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sla] + (anip1[sla] - ani[sla]) * t);
        mat_left_arm_upper = mat4x4_t::rotate(ani[ula] + (anip1[ula] - ani[ula]) * t);
        mat_left_arm_bottom = mat4x4_t::rotate(ani[lla] + (anip1[lla] - ani[lla]) * t);
        mat_right_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sra] + (anip1[sra] - ani[sra]) * t);
        mat_right_arm_upper = mat4x4_t::rotate(ani[ura] + (anip1[ura] - ani[ura]) * t);
        mat_right_arm_bottom = mat4x4_t::rotate(ani[lra] + (anip1[lra] - ani[lra]) * t);

        if (switch_sides)
        {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xl;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xr;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xal;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xar;
        }
        else
        {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xr;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xl;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xar;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xal;
        }
        if (mFacing == FACEDIR_UP)
        {
            set_transform(mat_right_arm_bottom * mat_elbow * mat_right_arm_upper);
            _draw_rect_in_n_out_(*arml, dbp == LowerRightArm ? hl : in, outline);
            set_transform(mat_left_arm_bottom * mat_elbow * mat_left_arm_upper);
            _draw_rect_in_n_out_(*arml, dbp == LowerLeftArm ? hl : in, outline);
        }
        if (!invert_order)
        {
            set_transform(mat_left_leg_upper);
            _draw_rect_in_n_out_(*legu, dbp == UpperLeftLeg ? hl : in, outline);
            set_transform(mat_left_leg_bottom * mat_knee * mat_left_leg_upper);
            _draw_rect_in_n_out_(*legl, dbp == LowerLeftLeg ? hl : in, outline);
        }
        set_transform(mat_right_leg_upper);
        _draw_rect_in_n_out_(*legu, dbp == UpperRightLeg ? hl : in, outline);
        set_transform(mat_right_leg_bottom * mat_knee * mat_right_leg_upper);
        _draw_rect_in_n_out_(*legl, dbp == LowerRightLeg ? hl : in, outline);
        if (invert_order)
        {
            set_transform(mat_left_leg_upper);
            _draw_rect_in_n_out_(*legu, dbp == UpperLeftLeg ? hl : in, outline);
            set_transform(mat_left_leg_bottom * mat_knee * mat_left_leg_upper);
            _draw_rect_in_n_out_(*legl, dbp == LowerLeftLeg ? hl : in, outline);
        }

        set_transform(mat_right_arm_upper);
        _draw_rect_in_n_out_(*armu, dbp == UpperRightArm ? hl : in, outline);
        set_transform(mat_left_arm_upper);
        _draw_rect_in_n_out_(*armu, dbp == UpperLeftArm ? hl : in, outline);

        set_transform(mat_bdy);
        _draw_rect_in_n_out_(body, dbp == Body ? hl : in, outline);
        set_transform(mat_head);
        _draw_rect_in_n_out_(head, dbp == Head ? hl : in, outline);

        if (mFacing == FACEDIR_DOWN)
        {
            set_transform(mat_right_arm_bottom * mat_elbow * mat_right_arm_upper);
            _draw_rect_in_n_out_(*arml, dbp == LowerRightArm ? hl : in, outline);
            set_transform(mat_left_arm_bottom * mat_elbow * mat_left_arm_upper);
            _draw_rect_in_n_out_(*arml, dbp == LowerLeftArm ? hl : in, outline);
        }
    }
    else
    {
        mat_left_leg_scale = mat4x4_t::scale(1.f, 1.f + ani[sll] + (anip1[sll] - ani[sll])*t);
        mat_left_leg_upper = mat4x4_t::rotate(ani[ull] + (anip1[ull] - ani[ull]) * t);
        mat_left_leg_bottom = mat4x4_t::rotate(ani[lll] + (anip1[lll] - ani[lll]) * t);

        mat_right_leg_scale = mat4x4_t::scale(1.f, 1.f + ani[srl] + (anip1[srl] - ani[srl])*t);
        mat_right_leg_upper = mat4x4_t::rotate(ani[url] + (anip1[url] - ani[url]) * t);
        mat_right_leg_bottom = mat4x4_t::rotate(ani[lrl] + (anip1[lrl] - ani[lrl]) * t);

        mat_left_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sla] + (anip1[sla] - ani[sla])*t);
        mat_left_arm_upper = mat4x4_t::rotate(ani[ula] + (anip1[ula] - ani[ula]) * t);
        mat_left_arm_bottom = mat4x4_t::rotate(ani[lla] + (anip1[lla] - ani[lla]) * t);
        mat_right_arm_scale = mat4x4_t::scale(1.f, 1.f + ani[sra] + (anip1[sra] - ani[sra])*t);
        mat_right_arm_upper = mat4x4_t::rotate(ani[ura] + (anip1[ura] - ani[ura]) * t);
        mat_right_arm_bottom = mat4x4_t::rotate(ani[lra] + (anip1[lra] - ani[lra]) * t);

        if (switch_sides)
        {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xl;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xr;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xal;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xar;
        }
        else
        {
            mat_left_leg_upper *= mat_left_leg_scale * dis_xr;
            mat_right_leg_upper *= mat_right_leg_scale * dis_xl;
            mat_left_arm_upper *= mat_left_arm_scale * dis_xar;
            mat_right_arm_upper *= mat_right_arm_scale * dis_xal;
        }
        set_transform(mat_left_arm_upper);
        _draw_rect_in_n_out_(*armu, dbp == UpperLeftArm ? hl : in, outline);
        set_transform(mat_left_arm_bottom * mat_elbow * mat_left_arm_upper);
        _draw_rect_in_n_out_(*arml, dbp == LowerLeftArm ? hl : in, outline);

        set_transform(mat_left_leg_upper);
        _draw_rect_in_n_out_(*legu, dbp == UpperLeftLeg ? hl : in, outline);
        set_transform(mat_left_leg_bottom * mat_knee * mat_left_leg_upper);
        _draw_rect_in_n_out_(*legl, dbp == LowerLeftLeg ? hl : in, outline);
        set_transform(mat_right_leg_upper);
        _draw_rect_in_n_out_(*legu, dbp == UpperRightLeg ? hl : in, outline);
        set_transform(mat_right_leg_bottom * mat_knee * mat_right_leg_upper);
        _draw_rect_in_n_out_(*legl, dbp == LowerRightLeg ? hl : in, outline);

        set_transform(mat_bdy);
        _draw_rect_in_n_out_(body, dbp == Body ? hl : in, outline);
        set_transform(mat_head);
        _draw_rect_in_n_out_(head, dbp == Head ? hl : in, outline);

        set_transform(mat_right_arm_upper);
        _draw_rect_in_n_out_(*armu, dbp == UpperRightArm ? hl : in, outline);
        set_transform(mat_right_arm_bottom * mat_elbow * mat_right_arm_upper);
        _draw_rect_in_n_out_(*arml, dbp == LowerRightArm ? hl : in, outline);
    }
    SetIdentity();
    /***********/
    return a + (t < 1.f ? t * .95f : .95f);
}
#endif

void Lamp::draw() const {
    set_transform(mat4x4_t::scale(mDrawScale) % mat4x4_t::translate(mX, mY) % parent.mCamera);
    draw_sprite(Database::instance->sprites[mSprite[0]]);
    SetIdentity();
}
#endif
